//#include "pch.h"
//
//#include <chrono>
//#include <thread>
//#include <cstring>
//#include "MemoryPool.h"
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//
//// 다양한 크기의 테스트 클래스
//class SmallObject {
//    int data;
//public:
//    SmallObject(int d = 0) : data(d) { /*std::cout << "SmallObject()" << std::endl;*/ }
//  /*  SmallObject(const SmallObject& other) : data(other.data) {
//        std::cout << "SmallObject() - 복사 생성자" << std::endl;
//    }
//    ~SmallObject()
//    {
//        std::cout << "~SmallObject()" << std::endl;
//    }*/
//    int getValue() const { return data; }
//};
//
//class MediumObject {
//    int data[32];  // 약 128바이트
//public:
//    MediumObject(int d = 0) {
//        for (int i = 0; i < 32; ++i) data[i] = d + i;
//    }
//    int getValue() const {
//        int sum = 0;
//        for (int i = 0; i < 32; ++i) sum += data[i];
//        return sum;
//    }
//};
//
//class LargeObject {
//    int data[256];  // 약 1024바이트
//public:
//    LargeObject(int d = 0) {
//        for (int i = 0; i < 256; ++i) data[i] = d + i;
//    }
//    int getValue() const {
//        int sum = 0;
//        for (int i = 0; i < 256; ++i) sum += data[i];
//        return sum;
//    }
//};
//
//// 멀티스레드 테스트
//void threadFunction(int id, int iterations) {
//    std::cout << "스레드 " << id << " 시작\n";
//
//    for (int i = 0; i < iterations; ++i) {
//        // 다양한 객체 할당 테스트
//        auto smallPtr = cnew<SmallObject>(i);
//        auto mediumPtr = cnew<MediumObject>(i);
//        auto largePtr = cnew<LargeObject>(i);
//
//        // 잠시 사용
//        int result = smallPtr->getValue() + mediumPtr->getValue() + largePtr->getValue();
//
//        // 해제
//        cdelete(smallPtr);
//        cdelete(mediumPtr);
//        cdelete(largePtr);
//
//        // 스마트 포인터 테스트
//        auto sharedSmall = MakeShared<SmallObject>(i);
//        auto sharedMedium = MakeShared<MediumObject>(i);
//    }
//
//    std::cout << "스레드 " << id << " 종료\n";
//
//    GlobalMemoryPool::ThreadLocalCacheClear();
//}
//
//
//int main()
//{
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//
//    {
//        Vector<SmallObject> v;
//
//        SmallObject s1(3);
//
//        v.push_back(s1);
//    }
//
//    std::cout << "글로벌 메모리 풀 테스트 시작\n";
//
//    const int NUM_THREADS = 8;
//    const int ITERATIONS_PER_THREAD = 1000000;
//
//    // 멀티스레딩 테스트
//    std::cout << "\n멀티스레딩 테스트 시작 (" << NUM_THREADS << " 스레드, 각 "
//        << ITERATIONS_PER_THREAD << " 반복)...\n";
//
//    std::vector<std::thread> threads;
//
//    auto start = std::chrono::high_resolution_clock::now();
//
//    for (int i = 0; i < NUM_THREADS; ++i) {
//        threads.emplace_back(threadFunction, i, ITERATIONS_PER_THREAD);
//    }
//
//    for (auto& t : threads) {
//        t.join();
//    }
//
//    auto end = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double> elapsed = end - start;
//
//    std::cout << "멀티스레딩 테스트 완료: " << elapsed.count() << "초\n";
//
//
//    GlobalMemoryPool::ThreadLocalCacheClear();
//	GlobalMemoryPool::Instance().Clear();
//
//	return 0;
//}

#include "pch.h"
#include "Profiler.h"
#include <iostream>
#include <thread>
#include <random>
#include <vector>

// 테스트용 작업 함수들
void DoHeavyCalculation()
{
    ScopedProfiler s("DoHeavyCalculation", __FUNCTION__, __LINE__);

    // 무거운 계산 시뮬레이션
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    int sum = 0;
    for (int i = 0; i < 1000000; ++i)
    {
        sum += dist(gen);
    }
}

//void ProcessData(int size)
//{
//    CUSTOM_PROFILE_FUNCTION();
//
//    // 일부 데이터 처리 시뮬레이션
//    for (int i = 0; i < size; ++i)
//    {
//        // 간단한 계산
//        if (i % 100 == 0)
//        {
//            CUSTOM_PROFILE_SCOPE("SubTask");
//            std::this_thread::sleep_for(std::chrono::microseconds(1));
//        }
//    }
//}
//
//void WorkerThread(int id, int iterations)
//{
//    // 스레드 작업 이름 생성
//    std::string threadName = "WorkerThread_" + std::to_string(id);
//    CUSTOM_PROFILE_SCOPE(threadName);
//
//    for (int i = 0; i < iterations; ++i)
//    {
//        if (i % 2 == 0)
//        {
//            DoHeavyCalculation();
//        }
//        else
//        {
//            ProcessData(10000);
//        }
//
//        // 약간의 휴식
//        std::this_thread::sleep_for(std::chrono::milliseconds(10));
//    }
//}

int main()
{
    setlocale(LC_ALL, "");

    // 프로파일러 설정
    GProfiler.Initialize(
        OutputMode::Both,   // 콘솔과 파일 모두에 출력
        1000,                               // 2초마다 출력
        "profiler_demo.log"                 // 로그 파일 이름
    );

    // 프로파일링 시작
    GProfiler.Start();

    {
        ScopedProfiler s("main", __FUNCTION__, __LINE__);

        std::cout << "프로파일러 테스트 시작..." << std::endl;

        // 여러 스레드 생성
        //std::vector<std::thread> threads;
        //const int threadCount = 4;
        //const int iterationsPerThread = 5;

        //for (int i = 0; i < threadCount; ++i)
        //{
        //    threads.emplace_back(WorkerThread, i, iterationsPerThread);
        //}

        // 메인 스레드에서도 일부 작업 수행
        for (int i = 0; i < 10; ++i)
        {
            ScopedProfiler s("MainThreadTask", __FUNCTION__, __LINE__);
            //ProcessData(5000);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        //// 모든 스레드 조인
        //for (auto& thread : threads)
        //{
        //    thread.join();
        //}

        std::cout << "모든 작업 완료" << std::endl;
    }

    // 프로파일링 중지 및 최종 결과 출력
    GProfiler.Stop();

    //// CSV로 결과 내보내기
    //if (CustomProfiler::GetInstance().ExportToCSV("profiler_results.csv"))
    //{
    //    std::cout << "CSV 파일로 결과 내보내기 성공" << std::endl;
    //}

    return 0;
}
