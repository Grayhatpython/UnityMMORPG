#include "Profiler.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <ctime>

Profiler GProfiler;

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
	threadProfileData->threadProfileDataLock.lock();

	//	Max Call Stack Depth 보다 작으면 Thread_local Thread Profile Data CallStack에 현재 depth에 name 저장
	if (threadProfileData->currentDepth < S_MAX_CALL_STACK_DEPTH)
		threadProfileData->callStack[threadProfileData->currentDepth] = name;

	//	현재 name에 대한 ProfileData 설정
	auto& profileData = (*threadProfileData->bufferPointer)[name];
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

	//	Call Stack Depth 감소
	if (threadProfileData->currentDepth > 0)
		threadProfileData->currentDepth--;

	//	name에 대한 Profile Data가 없다면?..
	auto it = (*threadProfileData->bufferPointer).find(name);
	if (it == (*threadProfileData->bufferPointer).end())
		return;

	//	name에 대한 Profile Data 설정
	auto& profileData = it->second;
	profileData.endTime = std::chrono::high_resolution_clock::now();
	profileData.durationTime = std::chrono::duration_cast<std::chrono::microseconds>(profileData.endTime - profileData.startTime);
	profileData.totalTime += profileData.durationTime;
	profileData.minDurationTime = std::min(profileData.minDurationTime, profileData.durationTime);
	profileData.maxDurationTime = std::max(profileData.maxDurationTime, profileData.durationTime);

	threadProfileData->threadProfileDataLock.unlock();
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
			threadProfileData->Clear();
	}
}

void Profiler::Print()
{
	//	_threadProfileDatas에 대한 Lock
	std::lock_guard<std::mutex> lock(_threadProfileLock);

	for (auto threadProfileData : _threadProfileDatas)
	{
		std::unordered_map<std::string, ProfileData>* readProfileDatesBuffer = nullptr;

		{
			//	각 스레드가 ProfileData를 기록중일 때는 Race Condition 피하기 위해 OutputThread는 기다린다.
			//	문제는 해당하는 스레드가 함수호출이 길어지면 로그가 밀리는 현상발생...
			//	그리고 해당하는 스레드가 중접 함수 호출 경우도 지금 같은 경우 recursive_mutex을 사용하여
			//	최초 호출한 함수가 recursive_mutex을 unlock 하지 않는 이상 기다린다..
			//	대부분 호출시간이 짧아서 괜찮을거 같다만 길찾기나 Ai나 Broadcast 코드부분에서 발생할수도 있다.
			//	그리고 output interval이 1000ms인데 위에서 얘기한데로 더 늦게 출력될수도 있다.
			// 
			//	그리고 중첩 함수 호출 중간에 로그를 안전하게 출력하면서 데이터를 유지하는 방법을 좀 생각해봐야겠다.
			//	중첩함수 호출같은 경우 중간에 함수가 어느정도 시간이 걸리는지 or Output Interval이 중첩함수 호출중간에 
			//	시간이 될 수 도 있기때문이다.
			threadProfileData->threadProfileDataLock.lock();

			//	포인터를 스왑하는 찰나의 순간만 Lock을 획득한다.
			//	writeBuffer Swap 하면서 readBuffer를 구해온다 -> readBuffer는 쓰기 없이 읽기만 하므로 Lock이 접근 가능
			threadProfileData->SwapProfileDatesBuffer(&readProfileDatesBuffer);

			threadProfileData->threadProfileDataLock.unlock();
		}

	}

	//	writeBuffer Swap으로 인해서 
	std::ostringstream oss;
	std::cout << "Test" << std::endl;
}

std::string Profiler::GetCallStack()
{
	//	Call Stack Depth Path 설정

	auto threadProfileData = GetThreadProfileData();

	std::string callStack;

	for (uint32_t i = 0; i < threadProfileData->currentDepth; ++i)
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

			Print();
		}
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



