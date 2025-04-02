#include "pch.h"
#include "ProfilerUI.h"
#include "Profiler.h"


void ProfilerUI::Initialize()
{
	if (_initialized == false)
	{
		_initialized = true;

		Profiler::GetInstance().Start();
	}
}

void ProfilerUI::Render()
{
	if (_visible == false || _initialized == false)
		return;

	if (ImGui::Begin("Profiler", &_visible, ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Show All Threads", nullptr, &_showAllThreads);
				ImGui::MenuItem("Show Event Details", nullptr, &_showEventDetails);
				ImGui::MenuItem("Show Frame Markers", nullptr, &_showFrameMarkers);
				ImGui::MenuItem("Auto Scroll", nullptr, &_autoScroll);
				ImGui::MenuItem("Show Filters", nullptr, &_showFilters);
				ImGui::EndMenu();
			}
		}

		if (ImGui::BeginMenu("Profiler")) 
		{
			if (ImGui::MenuItem("Start", nullptr, false, !Profiler::GetInstance().IsRunning())) 
				Profiler::GetInstance().Start();
			if (ImGui::MenuItem("Pause", nullptr, false, Profiler::GetInstance().IsRunning() && !Profiler::GetInstance().IsPaused())) 
				Profiler::GetInstance().Pause();
			if (ImGui::MenuItem("Resume", nullptr, false, Profiler::GetInstance().IsRunning() && Profiler::GetInstance().IsPaused())) 
				Profiler::GetInstance().Resume();
			if (ImGui::MenuItem("Stop", nullptr, false, Profiler::GetInstance().IsRunning())) 
				Profiler::GetInstance().Stop();

			ImGui::Separator();

			if (ImGui::MenuItem("Reset", nullptr)) 
			{
				Profiler::GetInstance().Reset();
				_selectedThreadIndex = -1;
				_selectedEventIndex = -1;
				_selectedFrameIndex = -1;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();


		// 컨트롤 UI 렌더링
		RenderControls();

		// 필터 UI 렌더링
		if (_showFilters) 
			RenderFilters();

		// 타임라인 렌더링
		ReenderTimeline();

		// 스레드 및 이벤트 렌더링
		RenderThreadsInfo();

		// 선택된 이벤트 정보 렌더링
		if (_showEventDetails && (_selectedThreadIndex >= 0 && _selectedEventIndex >= 0)) 
			RenderEventsInfo();
		

		// 선택된 프레임 정보 렌더링
		if (_showFrameMarkers && _selectedFrameIndex >= 0) 
			RenderFramesInfo();
	}

	ImGui::End();
}

void ProfilerUI::ReenderTimeline()
{
}

void ProfilerUI::RenderThreadsInfo()
{
}

void ProfilerUI::RenderFramesInfo()
{
}

void ProfilerUI::RenderEventsInfo()
{
}

void ProfilerUI::RenderFilters()
{
}

void ProfilerUI::RenderControls()
{
}

bool ProfilerUI::SelectEvent(float x, float y)
{
	return false;
}

std::string ProfilerUI::TimeToString(uint64 microseconds) const
{
	return std::string();
}

float ProfilerUI::TimeToScreen(uint64 time) const
{
	return 0.0f;
}

uint64 ProfilerUI::ScreenToTime(float position) const
{
	return uint64();
}

namespace Utils
{
	void SetUpHotkey(std::function<void()> toggleVisibleFunction)
	{
	}

	void AddMainMenuItem()
	{
	}

	void ProcessKeyInput(ImGuiIO& io)
	{

	}
}