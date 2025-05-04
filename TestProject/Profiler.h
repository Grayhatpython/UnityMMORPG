#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>
#include <array>
#include <fstream>
#include <sstream>
#include <functional>
#include <condition_variable>
#include <optional>

static constexpr size_t S_MAX_CALL_STACK_DEPTH = 64;

enum class OutputMode : uint32_t
{
	Console = 1,	//	콘솔 출력
	File = 2,		//	파일 출력
	Both = 3,
};

//	프로파일 데이터를 저장하는 구조체
struct ProfileData
{
	// 프로파일링 대상 코드 블록 또는 함수의 실행 시작 시점
	std::chrono::high_resolution_clock::time_point startTime;

	// 프로파일링 대상 코드 블록 또는 함수의 실행 종료 시점 (단일 호출 기준)
	std::chrono::high_resolution_clock::time_point endTime;

	// 해당 단일 호출의 실행 시간 (endTime - startTime)
	std::chrono::microseconds durationTime;

	// 프로파일링 대상이 총 누적 실행된 시간 (모든 호출의 durationTime 합계)
	std::chrono::microseconds totalTime;

	// 프로파일링 대상의 최소 실행 시간 (측정된 durationTime 중 가장 작은 값). 초기값은 microseconds가 가질 수 있는 최대값.
	std::chrono::microseconds minDurationTime{ std::chrono::microseconds::max() };

	// 프로파일링 대상의 최대 실행 시간 (측정된 durationTime 중 가장 큰 값). 초기값은 microseconds가 가질 수 있는 최소값.
	std::chrono::microseconds maxDurationTime{ std::chrono::microseconds::min() };

	// 이 프로파일 데이터가 기록된 시점까지의 함수 호출 경로 (콜 스택 정보)
	std::string callPath;

	// 프로파일링 대상이 호출된 총 횟수
	uint64_t callCount = 0;

	// 콜스택 깊이
	uint32_t callStackDepth = 0;

	// 이 프로파일 데이터가 기록된 스레드의 고유 ID
	std::thread::id theradID; // threadID 오타 수정 권장: threadID

};

//	스레드별 프로파일링 데이터
struct ThreadProfileData
{
	ThreadProfileData()
	{
		bufferPointer.store(&writeProfileDatesBuffer);
	}

	void SwapProfileDatesBuffer(std::unordered_map<std::string, ProfileData>** outReadProfileDatesBuffer)
	{
		//	현재 bufferPointer 즉, writeBuffer가 writeProfileDatesBuffer이면
		//	writeProfileDatesBuffer -> outReadProfileDatesBuffer로 즉 출력을 위한 ReadBuffer로
		//	bufferPointer 즉, writeBuffer를 readProfileDatesBuffer로 Swap
		if (bufferPointer == &writeProfileDatesBuffer)
		{
			*outReadProfileDatesBuffer = &writeProfileDatesBuffer;
			bufferPointer = &readProfileDatesBuffer;
		}
		//	현재 bufferPointer 즉, writeBuffer가 readProfileDatesBuffer이면
		//	readProfileDatesBuffer -> outReadProfileDatesBuffer로 즉 출력을 위한 ReadBuffer로
		//	bufferPointer 즉, writeBuffer를 writeProfileDatesBuffer로 Swap
		else
		{
			*outReadProfileDatesBuffer = &readProfileDatesBuffer;
			bufferPointer = &writeProfileDatesBuffer;
		}
	}

	void Clear()
	{
		currentDepth = 0;

		readProfileDatesBuffer.clear();
		writeProfileDatesBuffer.clear();

		bufferPointer.store(&writeProfileDatesBuffer);
	}

	std::recursive_mutex threadProfileDataLock;

	uint32_t currentDepth = 0;
	std::array<std::string, S_MAX_CALL_STACK_DEPTH> callStack;
	
	std::unordered_map<std::string, ProfileData> readProfileDatesBuffer;
	std::unordered_map<std::string, ProfileData> writeProfileDatesBuffer;
	std::atomic<std::unordered_map<std::string, ProfileData>*> bufferPointer;
	
};

class Profiler
{
public:
	Profiler();
	~Profiler();

public:

	/*
		프로파일링 초기화
		outputMode : 출력 모드 ( 콘솔 | 파일 )
		outputIntervalMilliseconds : 출력 인터벌 ( Milliseconds 기준 ) 
		fileName : 출력 파일 이름
	*/
	void Initialize(OutputMode outputMode = OutputMode::Console, uint16_t outputIntervalMilliseconds = 1000, const std::string& fileName = "Profile.log");

	/*
		프로파일링 시작
		name : 작업 이름
		filePath : 소스 파일 경로
		lineNumber : 소스 라인 번호
	*/
	void Begin(const std::string& name, const char* filePath = nullptr, int32_t lineNumber = -1);

	/*
		프로파일링 종료
		name : 작업 이름
	*/
	void End(const std::string& name);
	
public:
	//	프로파일링 시작
	void Start();

	//	프로파일링 중지
	void Stop();

	//	프로파일링 리셋
	void Reset();

	//	프로파일링 출력
	void Print();

public:
	//	스레드 콜스택 
	std::string			GetCallStack();

private:
	//	출력 스레드 업데이트 함수
	void				OutputThreadUpdate();

private:
	//	스레드 프로파일 데이터 
	ThreadProfileData*	GetThreadProfileData();
	//	현재 시간 문자열 
	std::string			GetNowTimeString();

private:
	//	thread_local 스레드별 데이터
	static thread_local ThreadProfileData* S_LThreadProfileData;

	//	통합 스레드별 데이터 접근 컨테이너
	std::vector<ThreadProfileData*> _threadProfileDatas;
	//	스레드별 데이터 추가/삭제를 위한 mutex
	std::mutex _threadProfileLock;
	std::atomic<bool>	_isRunning = false;
	
private:
	//	출력 스레드 관련
	std::thread			_outputThread;
	std::atomic<bool>	_shouldOutputNow = false;
	std::mutex			_outputLock;
	std::condition_variable _outputCv;

	//	출력 관련 설정
	OutputMode		_outputMode = OutputMode::Console;
	uint32_t		_outputIntervalMilliseconds = 1000;
	std::string		_outputFileName = "Profile.log";
	std::ofstream	_outputFile;
};

extern Profiler GProfiler;

// RAII(Resource Acquisition Is Initialization) 기반의 스코프 프로파일러
// 객체가 선언된 스코프(범위)를 벗어날 때 자동으로 프로파일링을 종료하고 결과를 기록합니다.
class ScopedProfiler
{
public:
	ScopedProfiler(const std::string& name, const char* filePath, int32_t lineNumber)
		: _name(name)
	{
		GProfiler.Begin(_name, filePath, lineNumber);
	}
	~ScopedProfiler()
	{
		GProfiler.End(_name);
	}

private:
	std::string _name;
};

