#include "pch.h"
#include "Exception.h"
#include <DbgHelp.h>
#include <TlHelp32.h>
#include <fstream>

#pragma comment(lib, "Dbghelp.lib")

void MakeDumpFile(EXCEPTION_POINTERS* exceptionInfo)
{
    const auto now = std::chrono::system_clock::now();
    time_t timeNow = std::chrono::system_clock::to_time_t(now);
    struct tm t;
    ::localtime_s(&t, &timeNow);

    WCHAR fileName[MAX_PATH];
    ::_snwprintf_s(fileName, len32(fileName), _TRUNCATE, L"%04d-%02d-%02d,%02d-%02d-%02d.dmp",
        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    try
    {
        //  덤프 파일 생성
        HANDLE file = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE,
            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (file == INVALID_HANDLE_VALUE)
        {
            std::cerr << "CreateFile Failed Error Code: " << GetLastError() << std::endl;
            return;
        }

        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = ::GetCurrentThreadId();
        mdei.ExceptionPointers = exceptionInfo;
        mdei.ClientPointers = FALSE;

        MINIDUMP_TYPE mdt = static_cast<MINIDUMP_TYPE>(MiniDumpWithPrivateReadWriteMemory |
            MiniDumpWithDataSegs |
            MiniDumpWithHandleData |
            MiniDumpWithFullMemoryInfo |
            MiniDumpWithThreadInfo |
            MiniDumpWithUnloadedModules);


        if (!::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), file, mdt,
            (exceptionInfo != nullptr) ? &mdei : nullptr, nullptr, nullptr)) {
            std::cerr << "MiniDumpWriteDump Failed Error Code : " << GetLastError() << std::endl;
        }

        if (file)
        {
            ::CloseHandle(file);
            file = INVALID_HANDLE_VALUE;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception occurred." << std::endl;
    }
}

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
    //  디버거가 실행 중이면 예외를 계속 검색하도록 반환
    if (IsDebuggerPresent())
        return EXCEPTION_CONTINUE_SEARCH;

    THREADENTRY32 te32;
    DWORD currentThreadId = ::GetCurrentThreadId();
    DWORD currentProcessId = ::GetCurrentProcessId();

    HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (snapshot != INVALID_HANDLE_VALUE)
    {
        te32.dwSize = sizeof(THREADENTRY32);

        if (::Thread32First(snapshot, &te32))
        {
            do
            {
                if (te32.th32OwnerProcessID == currentProcessId)
                {
                    if (te32.th32ThreadID != currentThreadId)
                    {
                        HANDLE thread = ::OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                        if (thread != NULL)
                        {
                            ::SuspendThread(thread);
                            //::CloseHandle(thread);
                        }
                    }
                }
            } while (::Thread32Next(snapshot, &te32));
        }

        ::CloseHandle(snapshot);
        snapshot = INVALID_HANDLE_VALUE;
    }

    MakeDumpFile(exceptionInfo);

    ::ExitProcess(TRUE);

    return EXCEPTION_EXECUTE_HANDLER;
}