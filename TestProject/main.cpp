//#include "pch.h"
//
//#include <chrono>
//#include <thread>
//#include <cstring>
//#include "MemoryPool.h"
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//
//// �پ��� ũ���� �׽�Ʈ Ŭ����
//class SmallObject {
//    int data;
//public:
//    SmallObject(int d = 0) : data(d) { /*std::cout << "SmallObject()" << std::endl;*/ }
//  /*  SmallObject(const SmallObject& other) : data(other.data) {
//        std::cout << "SmallObject() - ���� ������" << std::endl;
//    }
//    ~SmallObject()
//    {
//        std::cout << "~SmallObject()" << std::endl;
//    }*/
//    int getValue() const { return data; }
//};
//
//class MediumObject {
//    int data[32];  // �� 128����Ʈ
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
//    int data[256];  // �� 1024����Ʈ
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
//// ��Ƽ������ �׽�Ʈ
//void threadFunction(int id, int iterations) {
//    std::cout << "������ " << id << " ����\n";
//
//    for (int i = 0; i < iterations; ++i) {
//        // �پ��� ��ü �Ҵ� �׽�Ʈ
//        auto smallPtr = cnew<SmallObject>(i);
//        auto mediumPtr = cnew<MediumObject>(i);
//        auto largePtr = cnew<LargeObject>(i);
//
//        // ��� ���
//        int result = smallPtr->getValue() + mediumPtr->getValue() + largePtr->getValue();
//
//        // ����
//        cdelete(smallPtr);
//        cdelete(mediumPtr);
//        cdelete(largePtr);
//
//        // ����Ʈ ������ �׽�Ʈ
//        auto sharedSmall = MakeShared<SmallObject>(i);
//        auto sharedMedium = MakeShared<MediumObject>(i);
//    }
//
//    std::cout << "������ " << id << " ����\n";
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
//    std::cout << "�۷ι� �޸� Ǯ �׽�Ʈ ����\n";
//
//    const int NUM_THREADS = 8;
//    const int ITERATIONS_PER_THREAD = 1000000;
//
//    // ��Ƽ������ �׽�Ʈ
//    std::cout << "\n��Ƽ������ �׽�Ʈ ���� (" << NUM_THREADS << " ������, �� "
//        << ITERATIONS_PER_THREAD << " �ݺ�)...\n";
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
//    std::cout << "��Ƽ������ �׽�Ʈ �Ϸ�: " << elapsed.count() << "��\n";
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

// �׽�Ʈ�� �۾� �Լ���
void DoHeavyCalculation()
{
    ScopedProfiler s("DoHeavyCalculation", __FUNCTION__, __LINE__);

    // ���ſ� ��� �ùķ��̼�
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
//    // �Ϻ� ������ ó�� �ùķ��̼�
//    for (int i = 0; i < size; ++i)
//    {
//        // ������ ���
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
//    // ������ �۾� �̸� ����
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
//        // �ణ�� �޽�
//        std::this_thread::sleep_for(std::chrono::milliseconds(10));
//    }
//}

int main()
{
    setlocale(LC_ALL, "");

    // �������Ϸ� ����
    GProfiler.Initialize(
        OutputMode::Both,   // �ְܼ� ���� ��ο� ���
        1000,                               // 2�ʸ��� ���
        "profiler_demo.log"                 // �α� ���� �̸�
    );

    // �������ϸ� ����
    GProfiler.Start();

    {
        ScopedProfiler s("main", __FUNCTION__, __LINE__);

        std::cout << "�������Ϸ� �׽�Ʈ ����..." << std::endl;

        // ���� ������ ����
        //std::vector<std::thread> threads;
        //const int threadCount = 4;
        //const int iterationsPerThread = 5;

        //for (int i = 0; i < threadCount; ++i)
        //{
        //    threads.emplace_back(WorkerThread, i, iterationsPerThread);
        //}

        // ���� �����忡���� �Ϻ� �۾� ����
        for (int i = 0; i < 10; ++i)
        {
            ScopedProfiler s("MainThreadTask", __FUNCTION__, __LINE__);
            //ProcessData(5000);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        //// ��� ������ ����
        //for (auto& thread : threads)
        //{
        //    thread.join();
        //}

        std::cout << "��� �۾� �Ϸ�" << std::endl;
    }

    // �������ϸ� ���� �� ���� ��� ���
    GProfiler.Stop();

    //// CSV�� ��� ��������
    //if (CustomProfiler::GetInstance().ExportToCSV("profiler_results.csv"))
    //{
    //    std::cout << "CSV ���Ϸ� ��� �������� ����" << std::endl;
    //}

    return 0;
}
