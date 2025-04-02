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
    std::thread physicsThread(physicsThreadFunction);

    // 스레드 분리 (백그라운드로 실행)
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