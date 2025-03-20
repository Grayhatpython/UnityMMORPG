#pragma once
#include <chrono>
#include "SwapQueue.h"

inline void SetConsoleColor(WORD attributes) {
	HANDLE outputHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	::SetConsoleTextAttribute(outputHandle, attributes);
}

inline void ResetConsoleColor() {
	HANDLE outputHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	::SetConsoleTextAttribute(outputHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // 기본 색상 (흰색)
}

struct ThreadCallStackInfo
{
	std::string		funcName;
	std::string		fileName;
	int32			lineNumber = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

class ThreadHistory
{
public:
	void CallFunction(const std::string& funcName, const std::string& fileName, int32 lineNumber);
	void ReturnFunction();
	void Print();

private:
	void PushLog(const std::string& message);

private:
	SwapQueue<std::string>	_messageQueue;
};

class FunctionLog
{
public:
	FunctionLog(const std::string& funcName, const std::string& fileName, int32 lineNumber)
	{
		GThreadHistory->CallFunction(funcName, fileName, lineNumber);
	}
	~FunctionLog()
	{
		GThreadHistory->ReturnFunction();
	}
};

#define TRACE_THREAD_CALL_STACK FunctionLog __functionLog__(__FUNCTION__, __FILE__, __LINE__)
