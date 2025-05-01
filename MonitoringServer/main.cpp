/*
#include "pch.h"
#include "Profiler.h"
#include "ProfilerUI.h"

#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include <GLFW/glfw3.h>


// ������ ������ �Լ�
void renderThreadFunction() {
    PROFILE_THREAD("Render Thread");

    while (true) {
        PROFILE_FUNCTION();

        // ������ ����
        PROFILE_BEGIN_FRAME("Frame");

        // �ε��� ������ (����)
        {
            PROFILE_SCOPE("Shadow Depth Pass", "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

        // ���۵� ������ (����)
        {
            PROFILE_SCOPE("Deferred Pass", "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(3));

            // G-Buffer ����
            {
                PROFILE_SCOPE("G-Buffer", "Render");
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // ������ ���
            {
                PROFILE_SCOPE("Lighting", "Render");
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }

        // ����Ʈ ���μ��� (����)
        {
            PROFILE_SCOPE("Post Processing", "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // UI ������ (����)
        {
            PROFILE_SCOPE("UI Rendering", "Render");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // ������ ����
        PROFILE_END_FRAME();

        // 16ms ��� (�� 60fps)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// ���� ������ �Լ�
void physicsThreadFunction() {
    PROFILE_THREAD("Physics Thread");

    while (true) {
        PROFILE_FUNCTION();

        // �ݸ��� ���� (����)
        {
            PROFILE_SCOPE("Collision Detection", "Physics");
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }

        // ���� �ùķ��̼� (����)
        {
            PROFILE_SCOPE("Physics Simulation", "Physics");
            std::this_thread::sleep_for(std::chrono::milliseconds(4));

            // ������ �ٵ� ������Ʈ
            {
                PROFILE_SCOPE("Rigid Body Update", "Physics");
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }

            // ����Ʈ����Ʈ ���
            {
                PROFILE_SCOPE("Constraints", "Physics");
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        // 20ms ��� (�� 50fps)
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
}


int main() 
{
	if (::glfwInit() == GLFW_FALSE)
		return -1;

    // GLFW ������ ����
    const char* glsl_version = "#version 130";
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // ������ ����
    GLFWwindow* window = ::glfwCreateWindow(1280, 720, "Profiler Demo", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        ::glfwTerminate();
        return -1;
    }

    ::glfwMakeContextCurrent(window);
    ::glfwSwapInterval(1); // V-Sync Ȱ��ȭ

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

    // ��׶��� ������ ����
    std::thread renderThread(renderThreadFunction);
    //std::thread physicsThread(physicsThreadFunction);

    //// ������ �и� (��׶���� ����)
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

        // ImGui ������
        ImGui::Render();

        // OpenGL ������
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // ���� ��ü
        glfwSwapBuffers(window);
    }

    // ����
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


// ��ġ��ũ ���� ���� ����ü
struct BenchmarkResult {
    std::string name;
    double durationMs;
    size_t operationsCount;
    size_t threadCount;

    double getOperationsPerSecond() const {
        return operationsCount / (durationMs / 1000.0);
    }
};

// ���� ũ���� �޸� �Ҵ��� �����ϰ� �����ϴ� ��ũ�ε� �Լ�
void randomAllocationWorkload(int threadId, size_t iterations,
    std::function<void* (size_t)> allocFunc,
    std::function<void(void*)> freeFunc) {
    // ���� ������ �ʱ�ȭ
    std::mt19937 rng(std::random_device{}() + threadId);
    std::uniform_int_distribution<size_t> sizeDist(8, 2048);

    // �ӽ� ������ ���� ����
    std::vector<void*> pointers;
    pointers.reserve(iterations / 10); // �ִ� 10% ������ ����

    for (size_t i = 0; i < iterations; i++) {
        // ���� �Ҵ� ũ��
        size_t size = sizeDist(rng);

        // �޸� �Ҵ�
        void* ptr = allocFunc(size);

        // ������ �޸� ���� (ù ����Ʈ ����)
        if (ptr) {
            *static_cast<char*>(ptr) = static_cast<char>(i & 0xFF);
        }

        // ������ ����
        pointers.push_back(ptr);

        // �����ϰ� ���� �Ҵ� ���� (�� 80% Ȯ��)
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

    // ���� ������ ��� ����
    for (void* ptr : pointers) {
        if (ptr) {
            freeFunc(ptr);
        }
    }
}

// �⺻ malloc/free ��ũ�ε�
void mallocWorkload(int threadId, size_t iterations) {
    randomAllocationWorkload(threadId, iterations,
        [](size_t size) { return malloc(size); },
        [](void* ptr) { free(ptr); }
    );
}

// EnhancedMemoryPool ��ũ�ε�
void enhancedPoolWorkload(int threadId, size_t iterations) {
    randomAllocationWorkload(threadId, iterations,
        [](size_t size) { return GlobalMemoryManager::Allocate(size); },
        [](void* ptr) { GlobalMemoryManager::Deallocate(ptr); }
    );
}

// ������ ���ѵ� EnhancedMemoryPool ��ũ�ε�
void enhancedPoolLimitedWorkload(int threadId, size_t iterations) {
    // �����庰�� �ٸ� ũ�� ���� ��� (ĳ�� ȿ���� �׽�Ʈ)
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

// ���� ������ �޸� ��ũ�ε� (������ �� �޸� ����)
void crossThreadWorkload(const std::vector<std::thread::id>& threadIds, int threadId,
    size_t iterations, std::atomic<size_t>& nextThreadIndex,
    std::vector<std::mutex>& mutexes,
    std::vector<std::vector<void*>>& sharedPointers) {

    std::mt19937 rng(std::random_device{}() + threadId);
    std::uniform_int_distribution<size_t> sizeDist(8, 1024);

    // �ڽ��� ũ�� ���� (������ �� �ٸ� ũ��)
    const size_t minSize = 8 + (threadId * 16);
    const size_t maxSize = minSize + 256;

    // �ڽ��� ������ �ε���
    size_t myIndex = 0;
    for (size_t i = 0; i < threadIds.size(); i++) {
        if (threadIds[i] == std::this_thread::get_id()) {
            myIndex = i;
            break;
        }
    }

    for (size_t i = 0; i < iterations; i++) {
        // �Ҵ� �ܰ�
        size_t allocSize = minSize + (rng() % (maxSize - minSize));
        void* ptr = GlobalMemoryManager::Allocate(allocSize);

        // ������ �޸� �۾� ����
        if (ptr) {
            std::memset(ptr, static_cast<int>(i & 0xFF), std::min(allocSize, size_t(64)));
        }

        // �ڽ��� ������ ���Ϳ� �߰�
        {
            std::lock_guard<std::mutex> lock(mutexes[myIndex]);
            sharedPointers[myIndex].push_back(ptr);
        }

        // �ٸ� �������� �޸� ���� (���� ������ �ó�����)
        if (i % 10 == 0 && threadIds.size() > 1) {
            // �ٸ� ������ ����
            size_t targetIndex = (myIndex + 1) % threadIds.size();

            // �ٸ� �������� �޸� ����
            std::lock_guard<std::mutex> lock(mutexes[targetIndex]);
            if (!sharedPointers[targetIndex].empty()) {
                void* otherPtr = sharedPointers[targetIndex].back();
                sharedPointers[targetIndex].pop_back();

                // �ٸ� �����尡 �Ҵ��� �޸� ����
                GlobalMemoryManager::Deallocate(otherPtr);
            }
        }

        // ĳ�� ��ƿ�� ��� �׽�Ʈ�� ���� ������ Ȱ��ȭ/��Ȱ��ȭ
        if (i % 1000 == 0) {
            bool enableStealing = (i / 1000) % 2 == 0;
            GlobalMemoryManager::EnableCacheStealing(enableStealing);
        }
    }

    // �ڽ��� ���� ������ ����
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

// ���ü� ���� ��ũ�ε� (���ε༭-������ ����)
void producerConsumerWorkload(int threadId, bool isProducer,
    std::vector<std::mutex>& queueMutexes,
    std::vector<std::vector<void*>>& queues,
    std::atomic<bool>& shouldStop,
    std::atomic<size_t>& totalProduced,
    std::atomic<size_t>& totalConsumed) {

    const size_t queueCount = queueMutexes.size();
    std::mt19937 rng(std::random_device{}() + threadId);

    if (isProducer) {
        // ������ ����
        const size_t TARGET_PRODUCE = 100000; // �� ������ ��ǥ ���귮
        std::uniform_int_distribution<size_t> sizeDist(32, 256);

        for (size_t i = 0; i < TARGET_PRODUCE && !shouldStop; i++) {
            // �޸� �Ҵ�
            size_t size = sizeDist(rng);
            void* ptr = cnew<Knignt>();

            // ������ ������ ����
            if (ptr) {
                std::memset(ptr, static_cast<int>(i & 0xFF), std::min(size, size_t(32)));
            }

            // ���� ť ����
            size_t queueIndex = rng() % queueCount;

            // ť�� �ֱ�
            {
                std::lock_guard<std::mutex> lock(queueMutexes[queueIndex]);
                queues[queueIndex].push_back(ptr);
            }

            totalProduced.fetch_add(1, std::memory_order_relaxed);

            // ���� ��ũ�ε� �ùķ��̼��� ���� ª�� ������
            if (i % 1000 == 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
    }
    else {
        // �Һ��� ����
        while (!shouldStop || totalProduced != totalConsumed) {
            bool foundItem = false;

            // ���� ť ��ȸ
            size_t startQueueIndex = rng() % queueCount;

            for (size_t i = 0; i < queueCount && !foundItem; i++) {
                size_t queueIndex = (startQueueIndex + i) % queueCount;

                // ť���� �׸� �������� �õ�
                std::lock_guard<std::mutex> lock(queueMutexes[queueIndex]);
                if (!queues[queueIndex].empty()) {
                    void* ptr = queues[queueIndex].back();
                    queues[queueIndex].pop_back();

                    // ������ ������ �б�
                    if (ptr) {
                        char value = *static_cast<char*>(ptr);
                        // ���μ��� �ùķ��̼�
                        for (int j = 0; j < 10; j++) {
                            value = static_cast<char>(value * 1103515245 + 12345);
                        }

                        // �޸� ����
                        //GlobalMemoryManager::Deallocate(ptr);
                        cdelete(ptr);
                    }

                    totalConsumed.fetch_add(1, std::memory_order_relaxed);
                    foundItem = true;
                }
            }

            // �׸��� ã�� �������� ��� ���
            if (!foundItem) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
    }
}

// ��ġ��ũ ���� �Լ�
BenchmarkResult runBenchmark(const std::string& name,
    std::function<void(int, size_t)> workloadFunc,
    size_t threadCount, size_t iterationsPerThread) {

    std::vector<std::thread> threads;
    auto startTime = std::chrono::high_resolution_clock::now();

    // ������ ���� �� ����
    for (size_t i = 0; i < threadCount; i++) {
        threads.emplace_back(workloadFunc, static_cast<int>(i), iterationsPerThread);
    }

    // ��� ������ �Ϸ� ���
    for (auto& thread : threads) {
        thread.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;

    // ��� ��ȯ
    BenchmarkResult result;
    result.name = name;
    result.durationMs = duration.count();
    result.operationsCount = threadCount * iterationsPerThread;
    result.threadCount = threadCount;

    return result;
}

// ���� ������ ��ġ��ũ ���� �Լ�
BenchmarkResult runCrossThreadBenchmark(const std::string& name, size_t threadCount, size_t iterationsPerThread) {
    std::vector<std::thread> threads;
    std::vector<std::mutex> mutexes(threadCount);
    std::vector<std::vector<void*>> sharedPointers(threadCount);
    std::atomic<size_t> nextThreadIndex(0);
    std::vector<std::thread::id> threadIds(threadCount);

    // ���� ������ ID �ʱ�ȭ�� ���� ������ ����
    for (size_t i = 0; i < threadCount; i++) {
        threads.emplace_back([&threadIds, i]() {
            threadIds[i] = std::this_thread::get_id();
            });
    }

    // ������ ID �ʱ�ȭ �Ϸ� ���
    for (auto& thread : threads) {
        thread.join();
    }

    threads.clear();

    // ��ġ��ũ ����
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

    // ��� ��ȯ
    BenchmarkResult result;
    result.name = name;
    result.durationMs = duration.count();
    result.operationsCount = threadCount * iterationsPerThread;
    result.threadCount = threadCount;

    return result;
}

// ���ε༭-������ ��ġ��ũ ���� �Լ�
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

    // ��ġ��ũ ����
    auto startTime = std::chrono::high_resolution_clock::now();

    // ������ ������ ����
    for (size_t i = 0; i < producerCount; i++) {
        threads.emplace_back(producerConsumerWorkload, static_cast<int>(i), true,
            std::ref(queueMutexes), std::ref(queues),
            std::ref(shouldStop), std::ref(totalProduced),
            std::ref(totalConsumed));
    }

    // �Һ��� ������ ����
    for (size_t i = 0; i < consumerCount; i++) {
        threads.emplace_back(producerConsumerWorkload, static_cast<int>(producerCount + i), false,
            std::ref(queueMutexes), std::ref(queues),
            std::ref(shouldStop), std::ref(totalProduced),
            std::ref(totalConsumed));
    }

    // ���� �ð� ��� (�Ǵ� �ٸ� ���� ����)
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // ���� ��ȣ
    shouldStop.store(true);

    // ��� ������ �Ϸ� ���
    for (auto& thread : threads) {
        thread.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;

    // ��� ��ȯ
    BenchmarkResult result;
    result.name = name;
    result.durationMs = duration.count();
    result.operationsCount = totalProduced.load();
    result.threadCount = producerCount + consumerCount;

    return result;
}

// ��ġ��ũ ��� ��� �Լ�
void printBenchmarkResults(const std::vector<BenchmarkResult>& results) {
    // ��� ���
    std::cout << std::left << std::setw(40) << "��ġ��ũ �̸�"
        << std::right << std::setw(10) << "������"
        << std::right << std::setw(15) << "�ð�(ms)"
        << std::right << std::setw(15) << "�۾� ��"
        << std::right << std::setw(20) << "�ʴ� �۾� ��"
        << std::endl;

    std::cout << std::string(100, '-') << std::endl;

    // ��� ���
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
    // �⺻ �Ķ����
    const size_t ITERATIONS_PER_THREAD = 100000;
    std::vector<size_t> threadCounts = { 1, 2, 4, 8, 16 };

    std::vector<BenchmarkResult> results;

    std::cout << "=== �޸� �Ҵ��� ��Ƽ������ ��ġ��ũ ���� ===" << std::endl << std::endl;

    /*
    // 1. �⺻ �޸� �Ҵ� �� (malloc vs EnhancedMemoryPool)
    for (size_t threadCount : threadCounts) {
        // ���־�
        runBenchmark("���־�", mallocWorkload, 1, ITERATIONS_PER_THREAD / 10);

        // malloc/free ��ġ��ũ
        auto mallocResult = runBenchmark(
            "malloc/free - ���� ũ��",
            mallocWorkload,
            threadCount,
            ITERATIONS_PER_THREAD
        );
        results.push_back(mallocResult);

        // EnhancedMemoryPool ��ġ��ũ
        auto poolResult = runBenchmark(
            "EnhancedMemoryPool - ���� ũ��",
            enhancedPoolWorkload,
            threadCount,
            ITERATIONS_PER_THREAD
        );
        results.push_back(poolResult);

        // ���ѵ� ũ�� ���� EnhancedMemoryPool ��ġ��ũ
        auto limitedPoolResult = runBenchmark(
            "EnhancedMemoryPool - ���ѵ� ũ�� ����",
            enhancedPoolLimitedWorkload,
            threadCount,
            ITERATIONS_PER_THREAD
        );
        results.push_back(limitedPoolResult);

        std::cout << "������ " << threadCount << "�� �⺻ �� �׽�Ʈ �Ϸ�" << std::endl;
    }

    // 2. ���� ������ �׽�Ʈ (�ٸ� �����忡�� �Ҵ�/����)
    for (size_t threadCount : {2, 4, 8}) {
        auto crossThreadResult = runCrossThreadBenchmark(
            "���� ������ �޸� ����",
            threadCount,
            ITERATIONS_PER_THREAD / 2
        );
        results.push_back(crossThreadResult);

        std::cout << "������ " << threadCount << "�� ���� ������ �׽�Ʈ �Ϸ�" << std::endl;
    }
    */
    // 3. ���ε༭-������ ���� �׽�Ʈ
    for (auto [producers, consumers] : std::vector<std::pair<size_t, size_t>>{
        {1, 1}, {2, 2}, {4, 4}, {2, 6}, {4, 12}
        }) {
        auto pcResult = runProducerConsumerBenchmark(
            "���ε༭-������ ���� (P:" + std::to_string(producers) +
            ", C:" + std::to_string(consumers) + ")",
            producers,
            consumers,
            producers + consumers
        );
        results.push_back(pcResult);

        std::cout << "���ε༭ " << producers << "��, �Һ��� " << consumers
            << "�� ���ε༭-������ �׽�Ʈ �Ϸ�" << std::endl;
    }

    // ��� ��� ���
    std::cout << std::endl << "=== ��ġ��ũ ��� ��� ===" << std::endl << std::endl;
    printBenchmarkResults(results);

    // �޸� Ǯ ��� ���
    std::cout << std::endl << "=== �޸� Ǯ ��� ===" << std::endl;
    GlobalMemoryManager::PrintStats();

    return 0;
}