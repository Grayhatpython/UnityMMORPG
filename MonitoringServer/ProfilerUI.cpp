#include "pch.h"
#include "ProfilerUI.h"
#include "Profiler.h"
#include <sstream>
#include <iomanip>

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
			bool isRunning = Profiler::GetInstance().IsRunning();
			bool isPaused = Profiler::GetInstance().IsPaused();

			if (ImGui::MenuItem("Start", nullptr, false, !isRunning)) 
				Profiler::GetInstance().Start();
			if (ImGui::MenuItem("Pause", nullptr, false, isRunning && !isPaused))
				Profiler::GetInstance().Pause();
			if (ImGui::MenuItem("Resume", nullptr, false, isRunning && isPaused))
				Profiler::GetInstance().Resume();
			if (ImGui::MenuItem("Stop", nullptr, false, isRunning))
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
		RenderTimeline();

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

void ProfilerUI::RenderControls()
{
	//	 프레임 내부 여백을 가로 2, 세로 2 픽셀(또는 ImGui 내부 단위)로 설정
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	//	ControlsPanel 이라는 이름을 가지고, 높이가 100 픽셀로 고정되고, 부모 윈도우의 가로 공간을 꽉 채우며, 테두리가 있는 자식 윈도우를 생성
	ImGui::BeginChild("ControlsPanel", ImVec2(0, 100), true);

	{
		bool isRunning = Profiler::GetInstance().IsRunning();
		bool isPaused = Profiler::GetInstance().IsPaused();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Status:");
		ImGui::SameLine();

		if (isRunning == false)
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Stopped");
		else if (isPaused)
			ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Paused");
		else
			ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "Running");
		ImGui::SameLine(120);

		if (isRunning == false)
		{
			if (ImGui::Button("Start"))
				Profiler::GetInstance().Start();
		}
		else
		{
			if (isPaused)
			{
				if (ImGui::Button("Resume"))
					Profiler::GetInstance().Resume();
			}
			else
			{
				if (ImGui::Button("Pause"))
					Profiler::GetInstance().Pause();
			}

			ImGui::SameLine();

			if (ImGui::Button("Stop"))
				Profiler::GetInstance().Stop();
		}

		ImGui::SameLine();

		if (ImGui::Button("Reset"))
		{
			Profiler::GetInstance().Reset();
			_selectedThreadIndex = -1;
			_selectedEventIndex = -1;
			_selectedFrameIndex = -1;
		}

		ImGui::SameLine();

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Zoom:");
		ImGui::SameLine();

		if (ImGui::Button("-"))
			_zoom = std::max(0.1f, _zoom - 0.1f);
		ImGui::SameLine();

		if (ImGui::Button("+")) {
			_zoom = std::min(10.0f, _zoom + 0.1f);
		}
		ImGui::SameLine();

		ImGui::SetNextItemWidth(150.0f); // 슬라이더 너비를 150 픽셀로 설정
		float zoomValue = _zoom;
		if (ImGui::SliderFloat("##Zoom", &zoomValue, 0.1f, 10.0f, "%.1f"))
			_zoom = zoomValue;

		ImGui::SameLine();

		// 일시 정지 임계값
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Pause threshold:");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(150.0f); // 슬라이더 너비를 150 픽셀로 설정
		float pauseThreshold = static_cast<float>(Profiler::GetInstance().GetPauseThreshold()) / 1000.0f;
		if (ImGui::SliderFloat("##PauseThreshold", &pauseThreshold, 0.1f, 1000.0f, "%.1f ms"))
			Profiler::GetInstance().SetPauseThreshold(static_cast<uint64>(pauseThreshold * 1000.0f));

		ImGui::Spacing(); // 기본 간격 추가
		// 검색 필터
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Filter:");
		ImGui::SameLine();

		char searchBuffer[256];
		strncpy_s(searchBuffer, _searchFilter.c_str(), sizeof(searchBuffer));
		searchBuffer[sizeof(searchBuffer) - 1] = '\0';

		ImGui::SetNextItemWidth(150.0f); // 슬라이더 너비를 150 픽셀로 설정
		if (ImGui::InputText("##searchFilter", searchBuffer, sizeof(searchBuffer))) {
			_searchFilter = searchBuffer;
		}

		ImGui::SameLine();

		if (ImGui::Button("Clear"))
			_searchFilter.clear();

		ImGui::Checkbox("Auto scroll", &_autoScroll);
		ImGui::SameLine();
		ImGui::Checkbox("Show frames", &_showFrameMarkers);
		ImGui::SameLine();
		ImGui::Checkbox("Show details", &_showEventDetails);
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void ProfilerUI::RenderFilters()
{
	ImGui::BeginChild("FiltersPanel", ImVec2(0, 150), true);
	
	{
		ImGui::Columns(2);
		ImGui::Text("Threads:");
		ImGui::Separator();

		const auto& threadsInfo = Profiler::GetInstance().GetThreadsInfo();
		for (const auto& threadInfo : threadsInfo)
		{
			bool visible = Profiler::GetInstance().IsThreadVisible(threadInfo.name);
			if (ImGui::Checkbox(threadInfo.name.c_str(), &visible))
				Profiler::GetInstance().SetThreadFilter(threadInfo.name, visible);
		}

		ImGui::NextColumn();

		// 카테고리 필터
		ImGui::Text("Categories:");
		ImGui::Separator();

		// 카테고리 목록 (일반적으로 사용되는 몇 가지 카테고리)
		const char* categories[] = {
			"Render", "Compute", "IO", "Physics", "AI", "Audio", "Default", "Functions"
		};

		for (const char* category : categories) {
			bool visible = Profiler::GetInstance().IsCategoryVisible(category);
			if (ImGui::Checkbox(category, &visible)) {
				Profiler::GetInstance().SetCategoryFilter(category, visible);
			}
		}

	}

	ImGui::EndChild();
}

void ProfilerUI::RenderTimeline()
{
	
	const auto& framesInfo = Profiler::GetInstance().GetFramesInfo();

	if (framesInfo.empty() == true)
	{
		_timelineStart = 0;
		_timelineEnd = 0;
		_timelineRange = 0;
		return;
	}

	if (_autoScroll && framesInfo.empty() == false)
	{
        uint64 lastFrameEndTime = framesInfo.back().endTime;

		if (lastFrameEndTime > 0)
		{
			uint64 visibleRange = 5000000;

			_timelineStart = (lastFrameEndTime > visibleRange) ? lastFrameEndTime - visibleRange : 0;
			_timelineEnd = lastFrameEndTime;
			_timelineRange = _timelineEnd - _timelineStart;
		}
	}
	

	ImGui::BeginChild("TimelinePanel", ImVec2(0, _timelineHeight), true);
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImVec2 panelPos = ImGui::GetCursorScreenPos();
		ImVec2 panelSize = ImGui::GetContentRegionAvail();

		drawList->AddRectFilled(
			panelPos,
			ImVec2(panelPos.x + panelSize.x, panelPos.y + panelSize.y),
			IM_COL32(30, 30, 30, 255)
		);

		// 수직 눈금 간격 계산
		const int numGridLines = 10;
		float gridSpacing = panelSize.x / static_cast<float>(numGridLines);

		// 수직 눈금 그리기
		for (int i = 0; i <= numGridLines; ++i) {
			float x = panelPos.x + _threadLabelWidth + (i * gridSpacing);

			// 시간 텍스트 표시
			uint64_t timeAtMark = ScreenToTime(x - panelPos.x);
			std::string timeText = TimeToString(timeAtMark - _timelineStart);

			drawList->AddLine(
				ImVec2(x, panelPos.y),
				ImVec2(x, panelPos.y + panelSize.y),
				IM_COL32(70, 70, 70, 255),
				1.0f
			);

			// 시간 표시 (겹치지 않도록 간격 조정)
			if (i % 2 == 0) {
				drawList->AddText(
					ImVec2(x + 3, panelPos.y + 2),
					IM_COL32(180, 180, 180, 255),
					timeText.c_str()
				);
			}
		}

		// 프레임 마커 그리기
		if (_showFrameMarkers) {
			for (size_t i = 0; i < framesInfo.size(); ++i) {
				const auto& frame = framesInfo[i];

				// 화면에 표시될 범위를 벗어나면 스킵
				if (frame.startTime > _timelineEnd || frame.endTime < _timelineStart) {
					continue;
				}

				// 프레임 시작/종료 시간의 화면 위치 계산
				float startX = TimeToScreen(frame.startTime);
				float endX = TimeToScreen(frame.endTime);

				// 최소 표시 너비 설정 (1픽셀)
				float width = std::max(endX - startX, 1.0f);

				// 색상 설정 (선택된 프레임이면 다른 색상으로 표시)
				ImU32 frameColor = (static_cast<int>(i) == _selectedFrameIndex)
					? IM_COL32(255, 255, 0, 80)
					: IM_COL32(180, 180, 180, 80);

				// 프레임 배경 그리기
				drawList->AddRectFilled(
					ImVec2(panelPos.x + startX, panelPos.y),
					ImVec2(panelPos.x + startX + width, panelPos.y + panelSize.y),
					frameColor
				);

				// 프레임 경계선 그리기
				drawList->AddLine(
					ImVec2(panelPos.x + startX, panelPos.y),
					ImVec2(panelPos.x + startX, panelPos.y + panelSize.y),
					IM_COL32(180, 180, 180, 255),
					1.0f
				);

				// 프레임 인덱스 표시 (충분한 공간이 있을 때만)
				if (width > 20.0f) {
					std::string frameLabel = "F" + std::to_string(frame.index);

					drawList->AddText(
						ImVec2(panelPos.x + startX + 3, panelPos.y + 2),
						IM_COL32(255, 255, 255, 255),
						frameLabel.c_str()
					);
				}
			}
		}

	}
	ImGui::EndChild();
}

void ProfilerUI::RenderThreadsInfo()
{
}

void ProfilerUI::RenderEventsInfo()
{
}

void ProfilerUI::RenderFramesInfo()
{
}

bool ProfilerUI::SelectEvent(float x, float y)
{
	return false;
}

std::string ProfilerUI::TimeToString(uint64 microseconds) const
{
	std::stringstream ss;

	if (microseconds < 1000)
		ss << microseconds << " us";
	else if (microseconds < 1000000)
		ss << std::fixed << std::setprecision(2) << (microseconds / 1000.0f) << " ms";
	else
		ss << std::fixed << std::setprecision(2) << (microseconds / 1000000.0f) << " s";

	return ss.str();
}

float ProfilerUI::TimeToScreen(uint64 time) const
{
	if (_timelineRange == 0)
		return 0.0f;

	float relativeTime = static_cast<float>(time - _timelineStart) / static_cast<float>(_timelineRange);
	float screenWidth = ImGui::GetContentRegionAvail().x - _threadLabelWidth;

	return _threadLabelWidth + (relativeTime * screenWidth * _zoom) - (_panX * screenWidth);
}

uint64 ProfilerUI::ScreenToTime(float position) const
{
	float screenWidth = ImGui::GetContentRegionAvail().x - _threadLabelWidth;

	if (screenWidth <= 0.0f) return _timelineStart;

	float relativeX = (position - _threadLabelWidth + (_panX * screenWidth)) / (screenWidth * _zoom);
	uint64_t time = _timelineStart + static_cast<uint64_t>(relativeX * _timelineRange);

	return time;
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