#pragma once

#include "imgui/imgui.h"

class ProfilerUI
{
public:
	static ProfilerUI& GetInstance()
	{
		static ProfilerUI S_Instance;
		return S_Instance;
	}

public:	
	void Initialize();
	void Render();

private:
	void RenderControls();
	void RenderFilters();
	void RenderTimeline();
	void RenderThreadsInfo();
	void RenderEventsInfo();
	void RenderFramesInfo();

	bool		SelectEvent(float x, float y);
	std::string TimeToString(uint64 microseconds) const; 
	float		TimeToScreen(uint64 time) const;	
	uint64		ScreenToTime(float position) const;	

public:
	bool GetVisible() const { return _visible; }	
	void SetVisible(bool visible) { _visible = visible; }	
	void ToggleVisible() { _visible = !_visible; }	

private:
	bool _visible = false;
	bool _initialized = false;	

	uint64 _timelineStart = 0;
	uint64 _timelineEnd = 0;
	uint64 _timelineRange = 0;

	float _zoom = 1.0f;
	float _panX = 0.0f;	

	int32 _selectedThreadIndex = -1;
	int32 _selectedEventIndex = -1;
	int32 _selectedFrameIndex = -1;

	bool _showAllThreads = true;
	bool _showEventDetails = true;	
	bool _showFrameMarkers = true;
	bool _autoScroll = true;
	bool _showFilters = false;

	float _threadLabelWidth = 150.0f;
	float _eventHeight = 20.0f;
	float _timelineHeight = 40.0f;	
	float _threadSpacing = 5.0f;

	std::string _searchFilter;
};

namespace Utils
{
	void SetUpHotkey(std::function<void()> toggleVisibleFunction);
	void AddMainMenuItem();
	void ProcessKeyInput(ImGuiIO& io);
}