/*
#include "pch.h"
#include "Profiler.h"
#include "ProfilerUI.h"

#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include <GLFW/glfw3.h>


// 렌더링 스레드 함수
void renderThreadFunction() {
    PROFILE_THREAD("Render Thread");

    while (true) {
        PROFILE_FUNCTION();

        // 프레임 시작
        PROFILE_BEGIN_FRAME("Frame");

        // 셰도우 렌더링 (가정)
        {
            PROFILE_SCOPE("Shadow Depth Pass", "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

        // 디퍼드 렌더링 (가정)
        {
            PROFILE_SCOPE("Deferred Pass", "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(3));

            // G-Buffer 생성
            {
                PROFILE_SCOPE("G-Buffer", "Render");
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // 라이팅 계산
            {
                PROFILE_SCOPE("Lighting", "Render");
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }

        // 포스트 프로세싱 (가정)
        {
            PROFILE_SCOPE("Post Processing", "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // UI 렌더링 (가정)
        {
            PROFILE_SCOPE("UI Rendering", "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // 프레임 종료
        PROFILE_END_FRAME();

        // 16ms 대기 (약 60fps)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// 물리 스레드 함수
void physicsThreadFunction() {
    PROFILE_THREAD("Physics Thread");

    while (true) {
        PROFILE_FUNCTION();

        // 콜리전 감지 (가정)
        {
            PROFILE_SCOPE("Collision Detection", "Physics");
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }

        // 물리 시뮬레이션 (가정)
        {
            PROFILE_SCOPE("Physics Simulation", "Physics");
            std::this_thread::sleep_for(std::chrono::milliseconds(4));

            // 리지드 바디 업데이트
            {
                PROFILE_SCOPE("Rigid Body Update", "Physics");
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }

            // 컨스트레인트 계산
            {
                PROFILE_SCOPE("Constraints", "Physics");
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        // 20ms 대기 (약 50fps)
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
}


int main() 
{
	if (::glfwInit() == GLFW_FALSE)
		return -1;

    // GLFW 윈도우 설정
    const char* glsl_version = "#version 130";
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // 윈도우 생성
    GLFWwindow* window = ::glfwCreateWindow(1280, 720, "Profiler Demo", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        ::glfwTerminate();
        return -1;
    }

    ::glfwMakeContextCurrent(window);
    ::glfwSwapInterval(1); // V-Sync 활성화

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io; 
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   

	ImGui::StyleColorsDark();   

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ProfilerUI::GetInstance().Initialize();
    ProfilerUI::GetInstance().SetVisible(true);

    Utils::SetUpHotkey([]() {
            ProfilerUI::GetInstance().ToggleVisible();
        });

    PROFILE_THREAD("Main Thread");

    // 백그라운드 스레드 시작
    std::thread renderThread(renderThreadFunction);
    //std::thread physicsThread(physicsThreadFunction);

    //// 스레드 분리 (백그라운드로 실행)
    renderThread.detach();
    //physicsThread.detach();

    while (!glfwWindowShouldClose(window))
    {
        PROFILE_FUNCTION();

        ::glfwPollEvents();

        {
            PROFILE_SCOPE("Main Thread Work", "Compute");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        ::ImGui_ImplOpenGL3_NewFrame();
        ::ImGui_ImplGlfw_NewFrame();
        ::ImGui::NewFrame();

        Utils::ProcessKeyInput(io);
        Utils::AddMainMenuItem();

        ProfilerUI::GetInstance().Render();

        {
            PROFILE_SCOPE("ImGui Demo", "UI");

            ImGui::Begin("Demo");

            if (ImGui::Button("Toggle Profiler (F2"))
            {
                ProfilerUI::GetInstance().ToggleVisible();
            }

            ImGui::SameLine();

            static bool showDemoWindow = false;
            ImGui::Checkbox("Show ImGui Demo", &showDemoWindow);

            if (showDemoWindow) 
            {
                ImGui::ShowDemoWindow(&showDemoWindow);
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();
        }

        // ImGui 렌더링
        ImGui::Render();

        // OpenGL 렌더링
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 버퍼 교체
        glfwSwapBuffers(window);
    }

    // 정리
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
*/

#include "pch.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <random>
#include <memory>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <string>
#include "CustomMemoryPool.h"


// 벤치마크 정보 저장 구조체
struct BenchmarkResult {
    std::string name;
    double durationMs;
    size_t operationsCount;
    size_t threadCount;

    double getOperationsPerSecond() const {
        return operationsCount / (durationMs / 1000.0);
    }
};

// 여러 크기의 메모리 할당을 랜덤하게 수행하는 워크로드 함수
void randomAllocationWorkload(int threadId, size_t iterations,
    std::function<void* (size_t)> allocFunc,
    std::function<void(void*)> freeFunc) {
    // 랜덤 생성기 초기화
    std::mt19937 rng(std::random_device{}() + threadId);
    std::uniform_int_distribution<size_t> sizeDist(8, 2048);

    // 임시 포인터 저장 벡터
    std::vector<void*> pointers;
    pointers.reserve(iterations / 10); // 최대 10% 정도만 유지

    for (size_t i = 0; i < iterations; i++) {
        // 랜덤 할당 크기
        size_t size = sizeDist(rng);

        // 메모리 할당
        void* ptr = allocFunc(size);

        // 간단한 메모리 검증 (첫 바이트 쓰기)
        if (ptr) {
            *static_cast<char*>(ptr) = static_cast<char>(i & 0xFF);
        }

        // 포인터 저장
        pointers.push_back(ptr);

        // 랜덤하게 이전 할당 해제 (약 80% 확률)
        if (!pointers.empty() && (rng() % 100 < 80 || pointers.size() > iterations / 10)) {
            size_t index = rng() % pointers.size();
            void* oldPtr = pointers[index];

            if (oldPtr) {
                freeFunc(oldPtr);
            }

            pointers[index] = pointers.back();
            pointers.pop_back();
        }
    }

    // 남은 포인터 모두 해제
    for (void* ptr : pointers) {
        if (ptr) {
            freeFunc(ptr);
        }
    }
}

// 기본 malloc/free 워크로드
void mallocWorkload(int threadId, size_t iterations) {
    randomAllocationWorkload(threadId, iterations,
        [](size_t size) { return malloc(size); },
        [](void* ptr) { free(ptr); }
    );
}

// EnhancedMemoryPool 워크로드
void enhancedPoolWorkload(int threadId, size_t iterations) {
    randomAllocationWorkload(threadId, iterations,
        [](size_t size) { return GlobalMemoryManager::Allocate(size); },
        [](void* ptr) { GlobalMemoryManager::Deallocate(ptr); }
    );
}

// 범위로 제한된 EnhancedMemoryPool 워크로드
void enhancedPoolLimitedWorkload(int threadId, size_t iterations) {
    // 스레드별로 다른 크기 범위 사용 (캐시 효율성 테스트)
    const size_t minSize = 8 + (threadId * 8);
    const size_t maxSize = minSize + 256;

    std::mt19937 rng(std::random_device{}() + threadId);
    std::uniform_int_distribution<size_t> sizeDist(minSize, maxSize);

    std::vector<void*> pointers;
    pointers.reserve(iterations / 10);

    for (size_t i = 0; i < iterations; i++) {
        size_t size = sizeDist(rng);
        void* ptr = GlobalMemoryManager::Allocate(size);
        if (ptr) {
            *static_cast<char*>(ptr) = static_cast<char>(i & 0xFF);
        }

        pointers.push_back(ptr);

        if (!pointers.empty() && (rng() % 100 < 80 || pointers.size() > iterations / 10)) {
            size_t index = rng() % pointers.size();
            GlobalMemoryManager::Deallocate(pointers[index]);
            pointers[index] = pointers.back();
            pointers.pop_back();
        }
    }

    for (void* ptr : pointers) {
        GlobalMemoryManager::Deallocate(ptr);
    }
}

// 교차 스레드 메모리 워크로드 (스레드 간 메모리 전달)
void crossThreadWorkload(const std::vector<std::thread::id>& threadIds, int threadId,
    size_t iterations, std::atomic<size_t>& nextThreadIndex,
    std::vector<std::mutex>& mutexes,
    std::vector<std::vector<void*>>& sharedPointers) {

    std::mt19937 rng(std::random_device{}() + threadId);
    std::uniform_int_distribution<size_t> sizeDist(8, 1024);

    // 자신의 크기 범위 (스레드 간 다른 크기)
    const size_t minSize = 8 + (threadId * 16);
    const size_t maxSize = minSize + 256;

    // 자신의 스레드 인덱스
    size_t myIndex = 0;
    for (size_t i = 0; i < threadIds.size(); i++) {
        if (threadIds[i] == std::this_thread::get_id()) {
            myIndex = i;
            break;
        }
    }

    for (size_t i = 0; i < iterations; i++) {
        // 할당 단계
        size_t allocSize = minSize + (rng() % (maxSize - minSize));
        void* ptr = GlobalMemoryManager::Allocate(allocSize);

        // 간단한 메모리 작업 수행
        if (ptr) {
            std::memset(ptr, static_cast<int>(i & 0xFF), std::min(allocSize, size_t(64)));
        }

        // 자신의 포인터 벡터에 추가
        {
            std::lock_guard<std::mutex> lock(mutexes[myIndex]);
            sharedPointers[myIndex].push_back(ptr);
        }

        // 다른 스레드의 메모리 해제 (교차 스레드 시나리오)
        if (i % 10 == 0 && threadIds.size() > 1) {
            // 다른 스레드 선택
            size_t targetIndex = (myIndex + 1) % threadIds.size();

            // 다른 스레드의 메모리 해제
            std::lock_guard<std::mutex> lock(mutexes[targetIndex]);
            if (!sharedPointers[targetIndex].empty()) {
                void* otherPtr = sharedPointers[targetIndex].back();
                sharedPointers[targetIndex].pop_back();

                // 다른 스레드가 할당한 메모리 해제
                GlobalMemoryManager::Deallocate(otherPtr);
            }
        }

        // 캐시 스틸링 기능 테스트를 위한 간헐적 활성화/비활성화
        if (i % 1000 == 0) {
            bool enableStealing = (i / 1000) % 2 == 0;
            GlobalMemoryManager::EnableCacheStealing(enableStealing);
        }
    }

    // 자신의 남은 포인터 정리
    {
        std::lock_guard<std::mutex> lock(mutexes[myIndex]);
        for (void* ptr : sharedPointers[myIndex]) {
            GlobalMemoryManager::Deallocate(ptr);
        }
        sharedPointers[myIndex].clear();
    }
}

class Knignt
{
    size_t hp;
    size_t deff;
    size_t att;
    size_t mp;
};

// 동시성 패턴 워크로드 (프로듀서-컨슈머 패턴)
void producerConsumerWorkload(int threadId, bool isProducer,
    std::vector<std::mutex>& queueMutexes,
    std::vector<std::vector<void*>>& queues,
    std::atomic<bool>& shouldStop,
    std::atomic<size_t>& totalProduced,
    std::atomic<size_t>& totalConsumed) {

    const size_t queueCount = queueMutexes.size();
    std::mt19937 rng(std::random_device{}() + threadId);

    if (isProducer) {
        // 생산자 로직
        const size_t TARGET_PRODUCE = 100000; // 각 생산자 목표 생산량
        std::uniform_int_distribution<size_t> sizeDist(32, 256);

        for (size_t i = 0; i < TARGET_PRODUCE && !shouldStop; i++) {
            // 메모리 할당
            size_t size = sizeDist(rng);
            void* ptr = cnew<Knignt>();

            // 간단한 데이터 쓰기
            if (ptr) {
                std::memset(ptr, static_cast<int>(i & 0xFF), std::min(size, size_t(32)));
            }

            // 랜덤 큐 선택
            size_t queueIndex = rng() % queueCount;

            // 큐에 넣기
            {
                std::lock_guard<std::mutex> lock(queueMutexes[queueIndex]);
                queues[queueIndex].push_back(ptr);
            }

            totalProduced.fetch_add(1, std::memory_order_relaxed);

            // 실제 워크로드 시뮬레이션을 위한 짧은 딜레이
            if (i % 1000 == 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
    }
    else {
        // 소비자 로직
        while (!shouldStop || totalProduced != totalConsumed) {
            bool foundItem = false;

            // 랜덤 큐 순회
            size_t startQueueIndex = rng() % queueCount;

            for (size_t i = 0; i < queueCount && !foundItem; i++) {
                size_t queueIndex = (startQueueIndex + i) % queueCount;

                // 큐에서 항목 가져오기 시도
                std::lock_guard<std::mutex> lock(queueMutexes[queueIndex]);
                if (!queues[queueIndex].empty()) {
                    void* ptr = queues[queueIndex].back();
                    queues[queueIndex].pop_back();

                    // 간단한 데이터 읽기
                    if (ptr) {
                        char value = *static_cast<char*>(ptr);
                        // 프로세싱 시뮬레이션
                        for (int j = 0; j < 10; j++) {
                            value = static_cast<char>(value * 1103515245 + 12345);
                        }

                        // 메모리 해제
                        //GlobalMemoryManager::Deallocate(ptr);
                        cdelete(ptr);
                    }

                    totalConsumed.fetch_add(1, std::memory_order_relaxed);
                    foundItem = true;
                }
            }

            // 항목을 찾지 못했으면 잠시 대기
            if (!foundItem) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
    }
}

// 벤치마크 실행 함수
BenchmarkResult runBenchmark(const std::string& name,
    std::function<void(int, size_t)> workloadFunc,
    size_t threadCount, size_t iterationsPerThread) {

    std::vector<std::thread> threads;
    auto startTime = std::chrono::high_resolution_clock::now();

    // 스레드 생성 및 실행
    for (size_t i = 0; i < threadCount; i++) {
        threads.emplace_back(workloadFunc, static_cast<int>(i), iterationsPerThread);
    }

    // 모든 스레드 완료 대기
    for (auto& thread : threads) {
        thread.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;

    // 결과 반환
    BenchmarkResult result;
    result.name = name;
    result.durationMs = duration.count();
    result.operationsCount = threadCount * iterationsPerThread;
    result.threadCount = threadCount;

    return result;
}

// 교차 스레드 벤치마크 실행 함수
BenchmarkResult runCrossThreadBenchmark(const std::string& name, size_t threadCount, size_t iterationsPerThread) {
    std::vector<std::thread> threads;
    std::vector<std::mutex> mutexes(threadCount);
    std::vector<std::vector<void*>> sharedPointers(threadCount);
    std::atomic<size_t> nextThreadIndex(0);
    std::vector<std::thread::id> threadIds(threadCount);

    // 먼저 스레드 ID 초기화를 위한 스레드 생성
    for (size_t i = 0; i < threadCount; i++) {
        threads.emplace_back([&threadIds, i]() {
            threadIds[i] = std::this_thread::get_id();
            });
    }

    // 스레드 ID 초기화 완료 대기
    for (auto& thread : threads) {
        thread.join();
    }

    threads.clear();

    // 벤치마크 실행
    auto startTime = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < threadCount; i++) {
        threads.emplace_back(crossThreadWorkload, std::ref(threadIds), static_cast<int>(i),
            iterationsPerThread, std::ref(nextThreadIndex),
            std::ref(mutexes), std::ref(sharedPointers));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;

    // 결과 반환
    BenchmarkResult result;
    result.name = name;
    result.durationMs = duration.count();
    result.operationsCount = threadCount * iterationsPerThread;
    result.threadCount = threadCount;

    return result;
}

// 프로듀서-컨슈머 벤치마크 실행 함수
BenchmarkResult runProducerConsumerBenchmark(const std::string& name,
    size_t producerCount,
    size_t consumerCount,
    size_t queueCount) {
    std::vector<std::thread> threads;
    std::vector<std::mutex> queueMutexes(queueCount);
    std::vector<std::vector<void*>> queues(queueCount);
    std::atomic<bool> shouldStop(false);
    std::atomic<size_t> totalProduced(0);
    std::atomic<size_t> totalConsumed(0);

    // 벤치마크 실행
    auto startTime = std::chrono::high_resolution_clock::now();

    // 생산자 스레드 생성
    for (size_t i = 0; i < producerCount; i++) {
        threads.emplace_back(producerConsumerWorkload, static_cast<int>(i), true,
            std::ref(queueMutexes), std::ref(queues),
            std::ref(shouldStop), std::ref(totalProduced),
            std::ref(totalConsumed));
    }

    // 소비자 스레드 생성
    for (size_t i = 0; i < consumerCount; i++) {
        threads.emplace_back(producerConsumerWorkload, static_cast<int>(producerCount + i), false,
            std::ref(queueMutexes), std::ref(queues),
            std::ref(shouldStop), std::ref(totalProduced),
            std::ref(totalConsumed));
    }

    // 일정 시간 대기 (또는 다른 종료 조건)
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 종료 신호
    shouldStop.store(true);

    // 모든 스레드 완료 대기
    for (auto& thread : threads) {
        thread.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;

    // 결과 반환
    BenchmarkResult result;
    result.name = name;
    result.durationMs = duration.count();
    result.operationsCount = totalProduced.load();
    result.threadCount = producerCount + consumerCount;

    return result;
}

// 벤치마크 결과 출력 함수
void printBenchmarkResults(const std::vector<BenchmarkResult>& results) {
    // 헤더 출력
    std::cout << std::left << std::setw(40) << "벤치마크 이름"
        << std::right << std::setw(10) << "스레드"
        << std::right << std::setw(15) << "시간(ms)"
        << std::right << std::setw(15) << "작업 수"
        << std::right << std::setw(20) << "초당 작업 수"
        << std::endl;

    std::cout << std::string(100, '-') << std::endl;

    // 결과 출력
    for (const auto& result : results) {
        std::cout << std::left << std::setw(40) << result.name
            << std::right << std::setw(10) << result.threadCount
            << std::right << std::setw(15) << std::fixed << std::setprecision(2) << result.durationMs
            << std::right << std::setw(15) << result.operationsCount
            << std::right << std::setw(20) << std::fixed << std::setprecision(2)
            << result.getOperationsPerSecond()
            << std::endl;
    }
}

int main() {
    // 기본 파라미터
    const size_t ITERATIONS_PER_THREAD = 100000;
    std::vector<size_t> threadCounts = { 1, 2, 4, 8, 16 };

    std::vector<BenchmarkResult> results;

    std::cout << "=== 메모리 할당자 멀티스레드 벤치마크 시작 ===" << std::endl << std::endl;

    /*
    // 1. 기본 메모리 할당 비교 (malloc vs EnhancedMemoryPool)
    for (size_t threadCount : threadCounts) {
        // 워밍업
        runBenchmark("워밍업", mallocWorkload, 1, ITERATIONS_PER_THREAD / 10);

        // malloc/free 벤치마크
        auto mallocResult = runBenchmark(
            "malloc/free - 랜덤 크기",
            mallocWorkload,
            threadCount,
            ITERATIONS_PER_THREAD
        );
        results.push_back(mallocResult);

        // EnhancedMemoryPool 벤치마크
        auto poolResult = runBenchmark(
            "EnhancedMemoryPool - 랜덤 크기",
            enhancedPoolWorkload,
            threadCount,
            ITERATIONS_PER_THREAD
        );
        results.push_back(poolResult);

        // 제한된 크기 범위 EnhancedMemoryPool 벤치마크
        auto limitedPoolResult = runBenchmark(
            "EnhancedMemoryPool - 제한된 크기 범위",
            enhancedPoolLimitedWorkload,
            threadCount,
            ITERATIONS_PER_THREAD
        );
        results.push_back(limitedPoolResult);

        std::cout << "스레드 " << threadCount << "개 기본 비교 테스트 완료" << std::endl;
    }

    // 2. 교차 스레드 테스트 (다른 스레드에서 할당/해제)
    for (size_t threadCount : {2, 4, 8}) {
        auto crossThreadResult = runCrossThreadBenchmark(
            "교차 스레드 메모리 관리",
            threadCount,
            ITERATIONS_PER_THREAD / 2
        );
        results.push_back(crossThreadResult);

        std::cout << "스레드 " << threadCount << "개 교차 스레드 테스트 완료" << std::endl;
    }
    */
    // 3. 프로듀서-컨슈머 패턴 테스트
    for (auto [producers, consumers] : std::vector<std::pair<size_t, size_t>>{
        {1, 1}, {2, 2}, {4, 4}, {2, 6}, {4, 12}
        }) {
        auto pcResult = runProducerConsumerBenchmark(
            "프로듀서-컨슈머 패턴 (P:" + std::to_string(producers) +
            ", C:" + std::to_string(consumers) + ")",
            producers,
            consumers,
            producers + consumers
        );
        results.push_back(pcResult);

        std::cout << "프로듀서 " << producers << "개, 소비자 " << consumers
            << "개 프로듀서-컨슈머 테스트 완료" << std::endl;
    }

    // 결과 요약 출력
    std::cout << std::endl << "=== 벤치마크 결과 요약 ===" << std::endl << std::endl;
    printBenchmarkResults(results);

    // 메모리 풀 통계 출력
    std::cout << std::endl << "=== 메모리 풀 통계 ===" << std::endl;
    GlobalMemoryManager::PrintStats();

    return 0;
}