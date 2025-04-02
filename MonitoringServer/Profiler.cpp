#include "pch.h"
#include "Profiler.h"
#include <random>

Profiler::Profiler()
{
	std::array<float, 4> renderColor = { 0.0f, 0.8f, 0.0f, 1.0f };          // 녹색
	std::array<float, 4> computeColor = { 0.0f, 0.0f, 0.8f, 1.0f };        // 파란색
	std::array<float, 4> ioColor = { 0.8f, 0.0f, 0.0f, 1.0f };              // 빨간색
	std::array<float, 4> physicsColor = { 0.8f, 0.8f, 0.0f, 1.0f };        // 노란색
	std::array<float, 4> aiColor = { 0.8f, 0.0f, 0.8f, 1.0f };            // 보라색
	std::array<float, 4> audioColor = { 0.0f, 0.8f, 0.8f, 1.0f };          // 청록색
	std::array<float, 4> defaultColor = { 0.5f, 0.5f, 0.5f, 1.0f };        // 회색
	std::array<float, 4> functionColor = { 0.7f, 0.7f, 0.7f, 1.0f };      // 밝은 회색

	_categoryColors["Render"] = renderColor;
	_categoryColors["Compute"] = computeColor;
	_categoryColors["IO"] = ioColor;
	_categoryColors["Physics"] = physicsColor;
	_categoryColors["AI"] = aiColor;
	_categoryColors["Audio"] = audioColor;
	_categoryColors["Default"] = defaultColor;
	_categoryColors["Functions"] = functionColor;

	_threadFilters["Main"] = true;
	_categoryFilters["Render"] = true;
	_categoryFilters["Compute"] = true;
	_categoryFilters["IO"] = true;
	_categoryFilters["Physics"] = true;
	_categoryFilters["AI"] = true;
	_categoryFilters["Audio"] = true;
	_categoryFilters["Default"] = true;
	_categoryFilters["Functions"] = true;
}

Profiler::~Profiler()
{
	Stop();
}

void Profiler::BeginEvent(const std::string& name, const std::string& category)
{
	if (IsRunning() == false || IsPaused() == true)
		return;	

	std::lock_guard<std::mutex> lock(_lock);	

	uint32 threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());	
	int32 threadIndex = GetThreadIndex(threadID);	

	ProfileEvent event;
	event.name = name;	
	event.category = category;	
	event.threadID = threadID;	
	event.threadName = _threadsInfo[threadIndex].name;	
	event.startTime = GetCurrentTimestamp();	
	event.endTime = 0;	
	event.frameIndex = _currentFrameIndex;	

	event.depth = 0;
	auto& activeEvents = _activeEvents[threadID];	
	if (activeEvents.empty() == false)
	{
		auto& parentEvent = activeEvents.back();
		event.depth = parentEvent.depth + 1;
	}

	_maxDepth = std::max(_maxDepth, event.depth);

	SetProfileEventColor(event);	

	activeEvents.push_back(event);	
}

void Profiler::EndEvent(const std::string& name)
{
	if (IsRunning() == false || IsPaused() == true)
		return;

	std::lock_guard<std::mutex> lock(_lock);
	uint32 threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
	auto& activeEvents = _activeEvents[threadID];	

	if (activeEvents.empty() == true)
		return;	

	auto it = activeEvents.rbegin();	
	while (it != activeEvents.rend())
	{
		if (it->name == name)
		{
			ProfileEvent event = *it;
			event.endTime = GetCurrentTimestamp();	

			if(event.endTime - event.startTime < _pauseThreshold)
				_isPaused = true;

			int32 threadIndex = GetThreadIndex(threadID);	
			_threadsInfo[threadIndex].events.push_back(event);

			activeEvents.erase(std::next(it).base());
			break;
		}
		++it;
	}

}

void Profiler::BeginFrame(const std::string& name)
{
	if (IsRunning() == false || IsPaused() == true)
		return;

	std::lock_guard<std::mutex> lock(_lock);

	FrameInfo frameInfo;
	frameInfo.index = _currentFrameIndex;	
	frameInfo.name = name;	
	frameInfo.startTime = GetCurrentTimestamp();
	frameInfo.endTime = 0;

	_framesInfo.push_back(frameInfo);		
}

void Profiler::EndFrame()
{
	if (IsRunning() == false || IsPaused() == true)
		return;
	std::lock_guard<std::mutex> lock(_lock);
	
	if (_framesInfo.empty() == false && _framesInfo.back().index == _currentFrameIndex)
	{
		_framesInfo.back().endTime = GetCurrentTimestamp();
		_currentFrameIndex++;
	}
}

void Profiler::Start()
{
	std::lock_guard<std::mutex> lock(_lock);

	if (IsRunning() == false)
	{
		_isRunning = true;
		_isPaused = false;

		_startTimestamp = GetCurrentTimestamp();
		_currentFrameIndex = 0;

		_threadsInfo.clear();
		_framesInfo.clear();
		_activeEvents.clear();
	}
}

void Profiler::Pause()
{
	std::lock_guard<std::mutex> lock(_lock);

	if (IsRunning() == true && IsPaused() == false)
	{
		_isPaused = true;
		_pauseTimestamp = GetCurrentTimestamp();
	}	
}

void Profiler::Resume()
{
	std::lock_guard<std::mutex> lock(_lock);
	if (IsRunning() == true && IsPaused() == true)
		_isPaused = false;
}

void Profiler::Stop()
{
	std::lock_guard<std::mutex> lock(_lock);

	_isRunning = false;
	_isPaused = false;	
}

void Profiler::Reset()
{
	std::lock_guard<std::mutex> lock(_lock);
	_currentFrameIndex = 0;
	_threadsInfo.clear();
	_framesInfo.clear();
	_activeEvents.clear();

	if (IsRunning() == true)
	{
		_startTimestamp = GetCurrentTimestamp();
		_isPaused = false;
	}
}

uint64 Profiler::GetCurrentTimestamp() const
{
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

void Profiler::SetThreadName(uint32 threadID, const std::string& name)
{
	std::lock_guard<std::mutex> lock(_lock);

	int32 index = GetThreadIndex(threadID);	
	_threadsInfo[index].name = name;

	if (_threadFilters.find(name) == _threadFilters.end())
		_threadFilters[name] = true;
}

void Profiler::SetThreadFilter(const std::string& name, bool visible)
{
	std::lock_guard<std::mutex> lock(_lock);
	_threadFilters[name] = visible;
}

void Profiler::SetCategoryFilter(const std::string& category, bool visible)
{
	std::lock_guard<std::mutex> lock(_lock);
	_categoryFilters[category] = visible;	
}

bool Profiler::IsThreadVisible(const std::string& name) const
{
	auto findIt = _threadFilters.find(name);	
	return findIt != _threadFilters.end() ? findIt->second : true;
}

bool Profiler::IsCategoryVisible(const std::string& category) const
{
	auto findIt = _categoryFilters.find(category);
	return findIt != _categoryFilters.end() ? findIt->second : true;
}

int32 Profiler::GetThreadIndex(uint32 threadID) 
{
	for (auto i = 0; i < _threadsInfo.size(); i++)
	{
		if (_threadsInfo[i].id == threadID)
			return static_cast<int32>(i);	
	}	
	
	ThreadInfo threadInfo;
	threadInfo.id = threadID;
	threadInfo.name = "Thread " + std::to_string(threadID);
	threadInfo.orderIndex = static_cast<int32>(_threadsInfo.size());

	_threadsInfo.push_back(threadInfo);
	return static_cast<int32>(_threadsInfo.size() - 1);
}

void Profiler::SetProfileEventColor(ProfileEvent& event)
{
	auto findIt = _categoryColors.find(event.category);
	if (findIt != _categoryColors.end())
	{
		event.color = findIt->second;
	}
	else
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(0.3f, 0.9f);	

		std::array<float, 4> color = { dis(gen), dis(gen), dis(gen), 1.0f };	
		event.color = color;	
		_categoryColors[event.category] = color;	
	}
}
