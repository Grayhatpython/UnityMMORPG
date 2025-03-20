#include "Protocol.pb.h"

#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <atomic>
#include <deque>
#include <map>

// ImGui 관련 헤더
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "implot.h"

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

// 서버별 CPU 기록 데이터 구조체
struct ServerCPUData {
    std::string serverName;
    std::deque<float> cpuHistory;
    std::deque<int64_t> timeHistory;
    ImVec4 color;  // 서버별 그래프 색상
};

class MonitoringServer 
{
public:

    ~MonitoringServer() 
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    void Initialize() {
        // GLFW 초기화
        if (!glfwInit()) {
            std::cerr << "GLFW 초기화 실패" << std::endl;
            exit(EXIT_FAILURE);
        }


        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        // 윈도우 생성
        _window = glfwCreateWindow(1280, 720, "게임 서버 CPU 모니터링", NULL, NULL);
        if (_window == nullptr) {
            std::cerr << "GLFW 윈도우 생성 실패" << std::endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(_window);
        glfwSwapInterval(1); // vsync 활성화

        // ImGui 초기화
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(_window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

public:
    void SetCPUInfo(Protocol::CpuInfo& cpuInfo) 
    {
        std::lock_guard<std::mutex> lock(_dataMutex);

        Protocol::ServerType serverType = cpuInfo.servertype();
        // 새 서버면 데이터 구조 초기화
        if (_serverData.find(serverType) == _serverData.end()) 
        {
            ServerCPUData newData;
            newData.serverName = cpuInfo.servername();

            // 서버 ID에 기반한 색상 할당 (각 서버별로 다른 색상)
            float hue = (serverType * 0.618033988749895f);
            hue = hue - std::floor(hue);

            ImVec4 color;
            float r, g, b;
            ImGui::ColorConvertHSVtoRGB(hue, 0.6f, 0.9f, r, g, b);
            newData.color = ImVec4(r, g, b, 1.0f);

            _serverData[serverType] = newData;
        }

        // CPU 데이터 추가
        auto& data = _serverData[serverType];
        data.cpuHistory.push_back(cpuInfo.cpuusage());
        data.timeHistory.push_back(cpuInfo.timestamp());

        // 최대 데이터 포인트 수 유지
        if (data.cpuHistory.size() > MAX_HISTORY) {
            data.cpuHistory.pop_front();
            data.timeHistory.pop_front();
        }

        std::cout << "수신: 서버 " << cpuInfo.servername()
            << " (ID: " << cpuInfo.servertype()
            << "), CPU: " << cpuInfo.cpuusage() << "%" << std::endl;
        
    }

    void Update() 
    {
        while (!glfwWindowShouldClose(_window))
        {
            glfwPollEvents();

            // ImGui 프레임 시작
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            Render();

            // 렌더링
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(_window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(_window);
        }
    }

private:
    void Render() 
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_FirstUseEver);

        ImGui::Begin("게임 서버 CPU 모니터링", nullptr, ImGuiWindowFlags_NoCollapse);

        std::lock_guard<std::mutex> lock(_dataMutex);

        // 모든 서버의 CPU 사용량을 하나의 차트에 표시
        if (ImPlot::BeginPlot("CPU 사용량 (모든 서버)", ImVec2(-1, 400))) {
            ImPlot::SetupAxes("시간 (초)", "CPU 사용량 (%)",
                ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);  // CPU 사용량은 0-100%

            // 각 서버별 데이터 플롯
            for (auto& [serverID, data] : _serverData) {
                if (!data.cpuHistory.empty()) {
                    // 시간 축 조정 (상대적 시간 표시)
                    std::vector<float> times;
                    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();

                    for (const auto& timestamp : data.timeHistory) {
                        times.push_back(static_cast<float>(now - timestamp) / 1000.0f);  // 초 단위로 변환
                    }

                    // CPU 데이터를 벡터로 변환
                    std::vector<float> cpuValues(data.cpuHistory.begin(), data.cpuHistory.end());

                    // 플롯 그리기
                    ImPlot::PushStyleColor(ImPlotCol_Line, data.color);
                    ImPlot::PlotLine(data.serverName.c_str(), times.data(), cpuValues.data(), cpuValues.size());
                    ImPlot::PopStyleColor();
                }
            }
            ImPlot::EndPlot();
        }

        // 서버별 개별 CPU 차트
        for (auto& [serverID, data] : _serverData) {
            if (ImGui::CollapsingHeader(data.serverName.c_str())) {
                ImGui::Text("서버 ID: %u", serverID);

                if (!data.cpuHistory.empty()) {
                    ImGui::Text("현재 CPU 사용량: %.1f%%", data.cpuHistory.back());

                    // 평균 CPU 사용량 계산
                    float avgCPU = 0.0f;
                    for (const auto& cpu : data.cpuHistory) {
                        avgCPU += cpu;
                    }
                    avgCPU /= data.cpuHistory.size();
                    ImGui::Text("평균 CPU 사용량: %.1f%%", avgCPU);

                    // 최대/최소 CPU 사용량
                    float maxCPU = *std::max_element(data.cpuHistory.begin(), data.cpuHistory.end());
                    float minCPU = *std::min_element(data.cpuHistory.begin(), data.cpuHistory.end());
                    ImGui::Text("최대 CPU 사용량: %.1f%%", maxCPU);
                    ImGui::Text("최소 CPU 사용량: %.1f%%", minCPU);

                    // 개별 서버 그래프
                    std::string plotName = "CPU 사용량 - " + data.serverName;
                    if (ImPlot::BeginPlot(plotName.c_str(), ImVec2(-1, 200))) {
                        ImPlot::SetupAxes("시간 (초)", "CPU 사용량 (%)",
                            ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);

                        // 시간 축 조정
                        std::vector<float> times;
                        int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count();

                        for (const auto& timestamp : data.timeHistory) {
                            times.push_back(static_cast<float>(now - timestamp) / 1000.0f);
                        }

                        // CPU 데이터를 벡터로 변환
                        std::vector<float> cpuValues(data.cpuHistory.begin(), data.cpuHistory.end());

                        // 플롯 그리기
                        ImPlot::PushStyleColor(ImPlotCol_Line, data.color);
                        ImPlot::PlotLine("CPU", times.data(), cpuValues.data(), cpuValues.size());
                        ImPlot::PopStyleColor();

                        ImPlot::EndPlot();
                    }
                }
                else {
                    ImGui::Text("데이터가 없습니다.");
                }

                ImGui::Separator();
            }
        }

        ImGui::End();
    }

private:
        std::mutex _dataMutex;

        // 서버별 CPU 데이터 저장 (서버ID -> 데이터)
        std::map<Protocol::ServerType, ServerCPUData> _serverData;

        // 그래프에 표시할 최대 데이터 포인트 수
        const size_t MAX_HISTORY = 60;  // 60초 데이터

        // ImGui 관련 변수
        GLFWwindow* _window = nullptr;

};

extern MonitoringServer GMonitoringServer;