#include "Profiler.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <ctime>

thread_local ThreadProfileData* Profiler::S_LThreadProfileData = nullptr;

Profiler::Profiler()
{
	
}

Profiler::~Profiler()
{
	if (_isRunning.load() == true)
		Stop();

	if (_outputFile.is_open() == true)
		_outputFile.close();

	std::lock_guard<std::mutex> lock(_threadProfileLock);
	for (auto threadProfileData : _threadProfileDatas)
	{
		if (threadProfileData)
		{
			delete threadProfileData;
			threadProfileData = nullptr;
		}
	}

	_threadProfileDatas.clear();
}

void Profiler::Initialize(OutputMode outputMode, uint16_t outputIntervalMilliseconds, const std::string& fileName)
{
	_outputMode = outputMode;
	_outputIntervalMilliseconds = outputIntervalMilliseconds;
	_outputFileName = fileName;

	if (_outputMode == OutputMode::File || _outputMode == OutputMode::Both)
	{
		if (_outputFile.is_open())
			_outputFile.close();

		_outputFile.open(_outputFileName, std::ios::binary | std::ios::app);

		if (_outputFile.is_open() == false)
		{
			std::cerr << "Can't open Profile file : " << _outputFileName << std::endl;
			_outputMode = OutputMode::Console;
		}
		else
			_outputFile << " =========== Profile Start [ " << GetNowTimeString() << " ] =========== " << std::endl;
	}
}

void Profiler::Begin(const std::string& name, const char* filePath, int32_t lineNumber)
{
	if (_isRunning.load() == false)
		return; 

	//	각 Thread_local Thread Profile Data
	auto threadProfileData = GetThreadProfileData();
	auto pointProfileDatas = threadProfileData->pointProfilerDatas.load();

	//	Max Call Stack Depth 보다 작으면 Thread_local Thread Profile Data CallStack에 현재 depth에 name 저장
	if (threadProfileData->currentDepth < S_MAX_CALL_STACK_DEPTH)
		threadProfileData->callStack[threadProfileData->currentDepth] = name;

	//	현재 name에 대한 ProfileData 설정
	auto& profileData = (*pointProfileDatas)[name];
	profileData.startTime = std::chrono::high_resolution_clock::now();
	profileData.callStackDepth = threadProfileData->currentDepth;
	profileData.callCount++;
	profileData.callPath = GetCallStack();
	profileData.theradID = std::this_thread::get_id();

	threadProfileData->currentDepth++;
}

void Profiler::End(const std::string& name)
{
	if (_isRunning.load() == false)
		return;

	//	각 Thread_local Thread Profile Data
	auto threadProfileData = GetThreadProfileData();
	auto pointProfileDatas = threadProfileData->pointProfilerDatas.load();

	//	Call Stack Depth 감소
	if (threadProfileData->currentDepth > 0)
		threadProfileData->currentDepth--;

	//	name에 대한 Profile Data가 없다면?..
	auto it = pointProfileDatas->find(name);
	if (it == pointProfileDatas->end())
		return;

	//	name에 대한 Profile Data 설정
	auto profileData = it->second;
	profileData.endTime = std::chrono::high_resolution_clock::now();
	profileData.durationTime = std::chrono::duration_cast<std::chrono::microseconds>(profileData.endTime - profileData.startTime);
	profileData.totalTime += profileData.durationTime;
	profileData.minDurationTime = std::min(profileData.minDurationTime, profileData.durationTime);
	profileData.maxDurationTime = std::max(profileData.maxDurationTime, profileData.durationTime);
}

void Profiler::Start()
{
	if (_isRunning.load() == true)
		return;

	//	초기화
	Reset();

	_isRunning.store(true);

	//	출력 스레드 
	_outputThread = std::thread(&Profiler::OutputThreadUpdate, this);
}

void Profiler::Stop()
{
	if (_isRunning.load() == false)
		return;

	_isRunning.store(false);

	{
		std::lock_guard<std::mutex> lock(_outputLock);
		_shouldOutputNow.store(true);
		_outputCv.notify_one();  
	}

	if (_outputThread.joinable())
		_outputThread.join();

}

void Profiler::Reset()
{
	std::lock_guard<std::mutex> lock(_threadProfileLock);

	//	각 Thread_local Thread Profile Data 초기화
	for (auto threadProfileData : _threadProfileDatas)
	{
		if (threadProfileData)
		{
			threadProfileData->readProfileDatas.clear();
			threadProfileData->writeProfileDatas.clear();
			threadProfileData->currentDepth = 0;
		}
	}
}

void Profiler::Print()
{
	std::vector<std::pair<std::string, ProfileData>> profileDatas;

	for (auto threadProfileData : _threadProfileDatas)
	{
		auto pointProfileDatas = threadProfileData->pointProfilerDatas.load();

		//	타이밍이슈가 있다 무조건!!
		auto readProfieDatas = (pointProfileDatas == &threadProfileData->readProfileDatas) ? &threadProfileData->writeProfileDatas : &threadProfileData->readProfileDatas;

		for (const auto& [name, data] : *readProfieDatas)
		{
			profileDatas.emplace_back(name, data);
		}
	}

	std::ostringstream oss;
}

std::string Profiler::GetCallStack()
{
	//	Call Stack Depth Path 설정

	auto threadProfileData = GetThreadProfileData();

	std::string callStack;

	for (auto i = 0; i < threadProfileData->currentDepth; ++i)
	{
		if (i > 0)
			callStack += " -> ";
		callStack += threadProfileData->callStack[i];
	}

	return callStack;
}

void Profiler::OutputThreadUpdate()
{
	while (_isRunning.load() == true)
	{
		{
			std::unique_lock<std::mutex> lock(_outputLock);

			//	_outputIntervalMilliseconds 주기로 출력
			//	Stop() 함수 호출 시 _isRunning().stroe(false) -> _shouldOutputNow flag true 설정 -> _outputfCv notify_one -> Update 종료
			auto predicateTriggered = _outputCv.wait_for(lock, std::chrono::milliseconds(_outputIntervalMilliseconds), [this] { return _shouldOutputNow.load(); });

			if (predicateTriggered)
				_shouldOutputNow.store(false); // 출력 후 상태 초기화

			SwapThreadProfileDatasBuffer();
			Print();
		}
	}
}

void Profiler::SwapThreadProfileDatasBuffer()
{
	for (auto threadProfileData : _threadProfileDatas)
	{
		auto currentProfileDatas = threadProfileData->pointProfilerDatas.load();
		auto swapProfileDatas = (currentProfileDatas == &threadProfileData->readProfileDatas) ? &threadProfileData->writeProfileDatas : &threadProfileData->readProfileDatas;

		swapProfileDatas->clear();
		threadProfileData->pointProfilerDatas.store(swapProfileDatas);
	}
}

ThreadProfileData* Profiler::GetThreadProfileData()
{
	if (S_LThreadProfileData == nullptr)
	{
		S_LThreadProfileData = new ThreadProfileData();

		std::lock_guard<std::mutex> lock(_threadProfileLock);
		_threadProfileDatas.push_back(S_LThreadProfileData);
	}

	return S_LThreadProfileData;
}

std::string Profiler::GetNowTimeString()
{
	//	현재 시스템 시간
	auto now = std::chrono::system_clock::now();
	//	std::chrono::system_clock::time_point 형태의 시간을 C 스타일의 time_t 형태로 변환
	//	time_t는 1970년 1월 1일 00:00:00 협정 세계시 (UTC) 기준 경과된 초
	auto timeNow = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	//	날짜와 시간을 구성 요소별 ( 년, 월, 일, 시, 분, 초...) 로 저장하기 위한 tm 구조체 변수
	std::tm tm;
	//	time_t 형태의 시간('timeNow')을 현재 시스템의 로컬 시간대(Local Time)를 기준으로 년, 월, 일, 시, 분, 초 등으로 분해하여 'tm' 구조체에 저장
	::localtime_s(&tm, &timeNow);

	//	std::put_time 조작자를 사용하여 'tm' 구조체에 저장된 시간 정보를
	//  지정된 형식 문자열에 따라 포맷팅하여 stringstream 'ss'에 삽입
	//  "%Y-%m-%d %H:%M:%S"는 출력 형식 지정자
	ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
	return ss.str();
}



