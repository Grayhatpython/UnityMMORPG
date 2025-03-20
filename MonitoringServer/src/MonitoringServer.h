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

// ImGui ���� ���
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "implot.h"

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

// ������ CPU ��� ������ ����ü
struct ServerCPUData {
    std::string serverName;
    std::deque<float> cpuHistory;
    std::deque<int64_t> timeHistory;
    ImVec4 color;  // ������ �׷��� ����
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
        // GLFW �ʱ�ȭ
        if (!glfwInit()) {
            std::cerr << "GLFW �ʱ�ȭ ����" << std::endl;
            exit(EXIT_FAILURE);
        }


        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        // ������ ����
        _window = glfwCreateWindow(1280, 720, "���� ���� CPU ����͸�", NULL, NULL);
        if (_window == nullptr) {
            std::cerr << "GLFW ������ ���� ����" << std::endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(_window);
        glfwSwapInterval(1); // vsync Ȱ��ȭ

        // ImGui �ʱ�ȭ
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
        // �� ������ ������ ���� �ʱ�ȭ
        if (_serverData.find(serverType) == _serverData.end()) 
        {
            ServerCPUData newData;
            newData.serverName = cpuInfo.servername();

            // ���� ID�� ����� ���� �Ҵ� (�� �������� �ٸ� ����)
            float hue = (serverType * 0.618033988749895f);
            hue = hue - std::floor(hue);

            ImVec4 color;
            float r, g, b;
            ImGui::ColorConvertHSVtoRGB(hue, 0.6f, 0.9f, r, g, b);
            newData.color = ImVec4(r, g, b, 1.0f);

            _serverData[serverType] = newData;
        }

        // CPU ������ �߰�
        auto& data = _serverData[serverType];
        data.cpuHistory.push_back(cpuInfo.cpuusage());
        data.timeHistory.push_back(cpuInfo.timestamp());

        // �ִ� ������ ����Ʈ �� ����
        if (data.cpuHistory.size() > MAX_HISTORY) {
            data.cpuHistory.pop_front();
            data.timeHistory.pop_front();
        }

        std::cout << "����: ���� " << cpuInfo.servername()
            << " (ID: " << cpuInfo.servertype()
            << "), CPU: " << cpuInfo.cpuusage() << "%" << std::endl;
        
    }

    void Update() 
    {
        while (!glfwWindowShouldClose(_window))
        {
            glfwPollEvents();

            // ImGui ������ ����
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            Render();

            // ������
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

        ImGui::Begin("���� ���� CPU ����͸�", nullptr, ImGuiWindowFlags_NoCollapse);

        std::lock_guard<std::mutex> lock(_dataMutex);

        // ��� ������ CPU ��뷮�� �ϳ��� ��Ʈ�� ǥ��
        if (ImPlot::BeginPlot("CPU ��뷮 (��� ����)", ImVec2(-1, 400))) {
            ImPlot::SetupAxes("�ð� (��)", "CPU ��뷮 (%)",
                ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);  // CPU ��뷮�� 0-100%

            // �� ������ ������ �÷�
            for (auto& [serverID, data] : _serverData) {
                if (!data.cpuHistory.empty()) {
                    // �ð� �� ���� (����� �ð� ǥ��)
                    std::vector<float> times;
                    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();

                    for (const auto& timestamp : data.timeHistory) {
                        times.push_back(static_cast<float>(now - timestamp) / 1000.0f);  // �� ������ ��ȯ
                    }

                    // CPU �����͸� ���ͷ� ��ȯ
                    std::vector<float> cpuValues(data.cpuHistory.begin(), data.cpuHistory.end());

                    // �÷� �׸���
                    ImPlot::PushStyleColor(ImPlotCol_Line, data.color);
                    ImPlot::PlotLine(data.serverName.c_str(), times.data(), cpuValues.data(), cpuValues.size());
                    ImPlot::PopStyleColor();
                }
            }
            ImPlot::EndPlot();
        }

        // ������ ���� CPU ��Ʈ
        for (auto& [serverID, data] : _serverData) {
            if (ImGui::CollapsingHeader(data.serverName.c_str())) {
                ImGui::Text("���� ID: %u", serverID);

                if (!data.cpuHistory.empty()) {
                    ImGui::Text("���� CPU ��뷮: %.1f%%", data.cpuHistory.back());

                    // ��� CPU ��뷮 ���
                    float avgCPU = 0.0f;
                    for (const auto& cpu : data.cpuHistory) {
                        avgCPU += cpu;
                    }
                    avgCPU /= data.cpuHistory.size();
                    ImGui::Text("��� CPU ��뷮: %.1f%%", avgCPU);

                    // �ִ�/�ּ� CPU ��뷮
                    float maxCPU = *std::max_element(data.cpuHistory.begin(), data.cpuHistory.end());
                    float minCPU = *std::min_element(data.cpuHistory.begin(), data.cpuHistory.end());
                    ImGui::Text("�ִ� CPU ��뷮: %.1f%%", maxCPU);
                    ImGui::Text("�ּ� CPU ��뷮: %.1f%%", minCPU);

                    // ���� ���� �׷���
                    std::string plotName = "CPU ��뷮 - " + data.serverName;
                    if (ImPlot::BeginPlot(plotName.c_str(), ImVec2(-1, 200))) {
                        ImPlot::SetupAxes("�ð� (��)", "CPU ��뷮 (%)",
                            ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);

                        // �ð� �� ����
                        std::vector<float> times;
                        int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count();

                        for (const auto& timestamp : data.timeHistory) {
                            times.push_back(static_cast<float>(now - timestamp) / 1000.0f);
                        }

                        // CPU �����͸� ���ͷ� ��ȯ
                        std::vector<float> cpuValues(data.cpuHistory.begin(), data.cpuHistory.end());

                        // �÷� �׸���
                        ImPlot::PushStyleColor(ImPlotCol_Line, data.color);
                        ImPlot::PlotLine("CPU", times.data(), cpuValues.data(), cpuValues.size());
                        ImPlot::PopStyleColor();

                        ImPlot::EndPlot();
                    }
                }
                else {
                    ImGui::Text("�����Ͱ� �����ϴ�.");
                }

                ImGui::Separator();
            }
        }

        ImGui::End();
    }

private:
        std::mutex _dataMutex;

        // ������ CPU ������ ���� (����ID -> ������)
        std::map<Protocol::ServerType, ServerCPUData> _serverData;

        // �׷����� ǥ���� �ִ� ������ ����Ʈ ��
        const size_t MAX_HISTORY = 60;  // 60�� ������

        // ImGui ���� ����
        GLFWwindow* _window = nullptr;

};

extern MonitoringServer GMonitoringServer;