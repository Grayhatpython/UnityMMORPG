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
    std::thread physicsThread(physicsThreadFunction);

    // ������ �и� (��׶���� ����)
    renderThread.detach();
    physicsThread.detach();

    while (!glfwWindowShouldClose(window))
    {
        PROFILE_FUNCTION();

        ::glfwPollEvents();

        {
            PROFILE_SCOPE("Main Thread Work", "Compute");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
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