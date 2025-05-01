#pragma once

#include "Profiler.h"
#include <memory>
#include <unordered_map>

/**
 * @brief 기본 프로파일러를 확장한 고급 프로파일러 클래스
 * 
 * 기본 시간 측정 기능에 추가하여 메모리 사용량 추적과 CPU 사용률 측정 기능을 제공합니다.
 */
class AdvancedProfiler : public Profiler
{
public:
    /**
     * @brief 싱글톤 인스턴스 획득
     * @return AdvancedProfiler의 인스턴스
     */
    static AdvancedProfiler& GetInstance()
    {
        static AdvancedProfiler instance;
        return instance;
    }

    /**
     * @brief 메모리 사용량 측정 시작
     * @param tag 메모리 측정 태그
     */
    void BeginMemoryTrack(const std::string& tag);

    /**
     * @brief 메모리 사용량 측정 종료
     * @param tag 메모리 측정 태그
     * @return 사용된 메모리 크기 (바이트)
     */
    size_t EndMemoryTrack(const std::string& tag);

    /**
     * @brief CPU 사용률 측정 시작
     * @param tag CPU 측정 태그
     */
    void BeginCpuUsageTrack(const std::string& tag);

    /**
     * @brief CPU 사용률 측정 종료
     * @param tag CPU 측정 태그
     * @return CPU 사용률 (백분율, 0-100)
     */
    double EndCpuUsageTrack(const std::string& tag);

    /**
     * @brief 메모리 사용량 측정 결과 출력
     */
    void PrintMemoryResults();

    /**
     * @brief CPU 사용률 측정 결과 출력
     */
    void PrintCpuResults();

    /**
     * @brief 모든 측정 결과 출력 (시간, 메모리, CPU)
     */
    void PrintAllResults();

    /**
     * @brief 자동으로 scope를 벗어날 때 메모리 측정을 종료하는 객체
     */
    class ScopedMemoryProfiler
    {
    public:
        ScopedMemoryProfiler(const std::string& tag) : _tag(tag)
        {
            AdvancedProfiler::GetInstance().BeginMemoryTrack(_tag);
        }

        ~ScopedMemoryProfiler()
        {
            AdvancedProfiler::GetInstance().EndMemoryTrack(_tag);
        }

    private:
        std::string _tag;
    };

    /**
     * @brief 자동으로 scope를 벗어날 때 CPU 측정을 종료하는 객체
     */
    class ScopedCpuProfiler
    {
    public:
        ScopedCpuProfiler(const std::string& tag) : _tag(tag)
        {
            AdvancedProfiler::GetInstance().BeginCpuUsageTrack(_tag);
        }

        ~ScopedCpuProfiler()
        {
            AdvancedProfiler::GetInstance().EndCpuUsageTrack(_tag);
        }

    private:
        std::string _tag;
    };

private:
    AdvancedProfiler() = default;
    ~AdvancedProfiler() = default;
    AdvancedProfiler(const AdvancedProfiler&) = delete;
    AdvancedProfiler& operator=(const AdvancedProfiler&) = delete;

    struct MemoryTrackInfo
    {
        size_t startMemory{0};
        size_t endMemory{0};
        size_t peakMemory{0};
        size_t callCount{0};
        size_t totalMemoryUsed{0};
    };

    struct CpuTrackInfo
    {
        uint64_t startUserTime{0};
        uint64_t startKernelTime{0};
        uint64_t endUserTime{0};
        uint64_t endKernelTime{0};
        double cpuUsage{0.0};
        uint32_t callCount{0};
        double totalCpuUsage{0.0};
    };

    std::mutex _memoryMutex;
    std::mutex _cpuMutex;
    std::unordered_map<std::string, MemoryTrackInfo> _memoryTracks;
    std::unordered_map<std::string, CpuTrackInfo> _cpuTracks;

    // 현재 프로세스의 메모리 사용량을 가져옴 (플랫폼 의존적)
    size_t GetCurrentMemoryUsage();
    
    // 현재 프로세스의 CPU 시간을 가져옴 (플랫폼 의존적)
    void GetCpuTimes(uint64_t& userTime, uint64_t& kernelTime);
};

// 매크로를 통한 편리한 사용
#define PROFILE_MEMORY(tag) AdvancedProfiler::ScopedMemoryProfiler __memProfiler(tag)
#define PROFILE_CPU(tag) AdvancedProfiler::ScopedCpuProfiler __cpuProfiler(tag) 