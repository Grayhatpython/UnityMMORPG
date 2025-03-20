#include "pch.h"
#include "Log.h"
#include <string>
#include <iomanip>
#include <ctime>
#include <sstream>
#include "UtilityHelper.h"

void ThreadHistory::CallFunction(const std::string& funcName, const std::string& fileName, int32 lineNumber)
{
	LThreadCallStackInfo->push_back({ funcName, fileName, lineNumber, std::chrono::high_resolution_clock::now() });
	auto lastPos = fileName.find_last_of("/\\");

	PushLog("Call -> " + funcName + " ( " + fileName.substr(lastPos + 1) + ":" + std::to_string(lineNumber) + " )");
}

void ThreadHistory::ReturnFunction()
{
	if (LThreadCallStackInfo->empty() == false)
	{
		auto endTime = std::chrono::high_resolution_clock::now();
		auto callStackInfo = LThreadCallStackInfo->back();
		LThreadCallStackInfo->pop_back();

		auto elpasedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - callStackInfo.startTime).count();

		PushLog("Return -> " + callStackInfo.funcName + " Elapsed Time : " + std::to_string(elpasedTime) + " milliseconds.");
	}
}

void ThreadHistory::Print()
{
	std::queue<std::string> logs;
	_messageQueue.PopAll(logs);

	while (logs.empty() == false)
	{
		auto log = logs.front();
		logs.pop();

		std::cout << log;
	}
}

void ThreadHistory::PushLog(const std::string& message)
{
	auto now = std::chrono::system_clock::now();
	auto timeNow = std::chrono::system_clock::to_time_t(now);
	auto timeNowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	struct tm tm;
	::localtime_s(&tm, &timeNow);

	std::ostringstream timestamp;
	timestamp << std::put_time(&tm, " %Y-%m-%d %H:%M:%S");
	timestamp << '.' << std::setfill('0') << std::setw(3) << timeNowMs.count() << " | "; // 밀리초 추가

	PWSTR threadName = nullptr;
	::GetThreadDescription(::GetCurrentThread(), &threadName);
	auto name = UtilityHelper::ConvertUnicodeToUTF8(std::wstring(threadName));

	std::string log = timestamp.str() + std::string(LThreadCallStackInfo->size() * 2, ' ') + name + " [" + std::to_string(::GetCurrentThreadId()) + "] " + message + "\n"; // 깊이에 따른 들여쓰기
	_messageQueue.Push(log);
}
