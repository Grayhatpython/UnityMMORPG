#include "pch.h"
#include "AdvancedProfiler.h"
#include <iostream>
#include <iomanip>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

void AdvancedProfiler::BeginMemoryTrack(const std::string& tag)
{
    std::lock_guard<std::mutex> lock(_memoryMutex);
    
    auto& trackInfo = _memoryTracks[tag];
    trackInfo.startMemory = GetCurrentMemoryUsage();
}

size_t AdvancedProfiler::EndMemoryTrack(const std::string& tag)
{
    size_t currentMemory = GetCurrentMemoryUsage();
    
    std::lock_guard<std::mutex> lock(_memoryMutex);
    
    auto it = _memoryTracks.find(tag);
    if (it == _memoryTracks.end())
        return 0;
    
    auto& trackInfo = it->second;
    trackInfo.endMemory = currentMemory;
    size_t memoryUsed = trackInfo.endMemory > trackInfo.startMemory ? 
                         trackInfo.endMemory - trackInfo.startMemory : 0;
    
    trackInfo.callCount++;
    trackInfo.totalMemoryUsed += memoryUsed;
    trackInfo.peakMemory = std::max(trackInfo.peakMemory, memoryUsed);
    
    return memoryUsed;
}

void AdvancedProfiler::BeginCpuUsageTrack(const std::string& tag)
{
    std::lock_guard<std::mutex> lock(_cpuMutex);
    
    auto& trackInfo = _cpuTracks[tag];
    GetCpuTimes(trackInfo.startUserTime, trackInfo.startKernelTime);
}

double AdvancedProfiler::EndCpuUsageTrack(const std::string& tag)
{
    uint64_t endUserTime = 0;
    uint64_t endKernelTime = 0;
    GetCpuTimes(endUserTime, endKernelTime);
    
    std::lock_guard<std::mutex> lock(_cpuMutex);
    
    auto it = _cpuTracks.find(tag);
    if (it == _cpuTracks.end())
        return 0.0;
    
    auto& trackInfo = it->second;
    trackInfo.endUserTime = endUserTime;
    trackInfo.endKernelTime = endKernelTime;
    
    // 시작 시간부터 종료 시간까지의 CPU 사용량 계산 (사용자 + 커널 시간)
    uint64_t userTimeDiff = trackInfo.endUserTime - trackInfo.startUserTime;
    uint64_t kernelTimeDiff = trackInfo.endKernelTime - trackInfo.startKernelTime;
    uint64_t totalTimeDiff = userTimeDiff + kernelTimeDiff;
    
    // 100us 단위를 백분율로 변환 (Windows 특화)
    auto cpuTime = high_resolution_clock::now() - high_resolution_clock::now();
    auto elapsedMs = duration_cast<milliseconds>(cpuTime).count();
    
    if (elapsedMs > 0)
    {
        // CPU 사용률 계산 (Windows에서는 100us 단위로 반환됨)
        trackInfo.cpuUsage = (totalTimeDiff / 10000.0) / elapsedMs * 100.0;
    }
    else
    {
        trackInfo.cpuUsage = 0.0;
    }
    
    trackInfo.callCount++;
    trackInfo.totalCpuUsage += trackInfo.cpuUsage;
    
    return trackInfo.cpuUsage;
}

void AdvancedProfiler::PrintMemoryResults()
{
    std::lock_guard<std::mutex> lock(_memoryMutex);
    
    std::cout << "======================= Memory Usage Results =======================" << std::endl;
    std::cout << std::setw(25) << "TAG" << " | ";
    std::cout << std::setw(10) << "CALLS" << " | ";
    std::cout << std::setw(15) << "TOTAL (KB)" << " | ";
    std::cout << std::setw(15) << "AVG (KB)" << " | ";
    std::cout << std::setw(15) << "PEAK (KB)" << std::endl;
    std::cout << "----------------------------------------------------------------" << std::endl;
    
    for (const auto& pair : _memoryTracks)
    {
        const auto& tag = pair.first;
        const auto& info = pair.second;
        
        if (info.callCount == 0)
            continue;
        
        double totalKb = info.totalMemoryUsed / 1024.0;
        double avgKb = totalKb / info.callCount;
        double peakKb = info.peakMemory / 1024.0;
        
        std::cout << std::setw(25) << tag << " | ";
        std::cout << std::setw(10) << info.callCount << " | ";
        std::cout << std::setw(15) << std::fixed << std::setprecision(2) << totalKb << " | ";
        std::cout << std::setw(15) << std::fixed << std::setprecision(2) << avgKb << " | ";
        std::cout << std::setw(15) << std::fixed << std::setprecision(2) << peakKb << std::endl;
    }
    
    std::cout << "================================================================" << std::endl;
}

void AdvancedProfiler::PrintCpuResults()
{
    std::lock_guard<std::mutex> lock(_cpuMutex);
    
    std::cout << "======================= CPU Usage Results =========================" << std::endl;
    std::cout << std::setw(25) << "TAG" << " | ";
    std::cout << std::setw(10) << "CALLS" << " | ";
    std::cout << std::setw(15) << "AVG CPU (%)" << std::endl;
    std::cout << "----------------------------------------------------------------" << std::endl;
    
    for (const auto& pair : _cpuTracks)
    {
        const auto& tag = pair.first;
        const auto& info = pair.second;
        
        if (info.callCount == 0)
            continue;
        
        double avgCpuUsage = info.totalCpuUsage / info.callCount;
        
        std::cout << std::setw(25) << tag << " | ";
        std::cout << std::setw(10) << info.callCount << " | ";
        std::cout << std::setw(15) << std::fixed << std::setprecision(2) << avgCpuUsage << std::endl;
    }
    
    std::cout << "================================================================" << std::endl;
}

void AdvancedProfiler::PrintAllResults()
{
    // 기본 시간 프로파일링 결과 출력
    PrintResults();
    
    // 메모리 사용량 결과 출력
    PrintMemoryResults();
    
    // CPU 사용량 결과 출력
    PrintCpuResults();
}

size_t AdvancedProfiler::GetCurrentMemoryUsage()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
    {
        // WorkingSetSize: 현재 실제 메모리에 적재된 크기 (RSS)
        return static_cast<size_t>(pmc.WorkingSetSize);
    }
    return 0;
#else
    // Linux 환경에서의 구현 (예시)
    FILE* file = fopen("/proc/self/statm", "r");
    if (file)
    {
        long rss = 0;
        fscanf(file, "%*s %ld", &rss);
        fclose(file);
        return static_cast<size_t>(rss * sysconf(_SC_PAGESIZE));
    }
    return 0;
#endif
}

void AdvancedProfiler::GetCpuTimes(uint64_t& userTime, uint64_t& kernelTime)
{
#ifdef _WIN32
    FILETIME creationTime, exitTime, kernelTimeFile, userTimeFile;
    if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTimeFile, &userTimeFile))
    {
        ULARGE_INTEGER kernelTimeInt, userTimeInt;
        kernelTimeInt.LowPart = kernelTimeFile.dwLowDateTime;
        kernelTimeInt.HighPart = kernelTimeFile.dwHighDateTime;
        userTimeInt.LowPart = userTimeFile.dwLowDateTime;
        userTimeInt.HighPart = userTimeFile.dwHighDateTime;
        
        kernelTime = kernelTimeInt.QuadPart;
        userTime = userTimeInt.QuadPart;
    }
    else
    {
        kernelTime = 0;
        userTime = 0;
    }
#else
    // Linux 환경에서의 구현 (예시)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        // 마이크로초 단위를 100ns 단위로 변환 (Windows와 호환성 유지)
        userTime = (usage.ru_utime.tv_sec * 1000000LL + usage.ru_utime.tv_usec) * 10;
        kernelTime = (usage.ru_stime.tv_sec * 1000000LL + usage.ru_stime.tv_usec) * 10;
    }
    else
    {
        userTime = 0;
        kernelTime = 0;
    }
#endif
} 