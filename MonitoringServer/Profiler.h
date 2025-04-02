#pragma once

struct ProfileEvent
{
	std::string name;
	std::string category;
	std::string threadName;

	uint32 threadID = 0;
	uint32 depth = 0;
	uint32 frameIndex = 0;
	
	uint64 startTime = 0;
	uint64 endTime = 0;	

	std::array<float, 4> color;
};

struct ThreadInfo
{
	uint32 id = 0;
	int32 orderIndex = 0;
	std::string name;

	std::vector<ProfileEvent> events;
};	

struct FrameInfo
{
	uint32 index = 0;
	uint64 startTime = 0;
	uint64 endTime = 0;
	std::string name;
};

class Profiler
{
private:
	Profiler();
	~Profiler();

public:
	static Profiler& GetInstance()
	{
		static Profiler S_Instance;
		return S_Instance;
	}

public:
	void BeginEvent(const std::string& name, const std::string& category = "Default");
	void EndEvent(const std::string& name);

	void BeginFrame(const std::string& name = "Frame");
	void EndFrame();	

	void Start();
	void Pause();
	void Resume();
	void Stop();	
	void Reset();	

public:
	const std::vector<ThreadInfo>& GetThreadsInfo() const { return _threadsInfo; }
	const std::vector<FrameInfo>& GetFramesInfo() const { return _framesInfo; }

	uint32 GetCurrentFrameIndex() const { return _currentFrameIndex; }
	uint64 GetCurrentTimestamp() const;	

	void SetThreadName(uint32 threadID, const std::string& name);

	void SetThreadFilter(const std::string& name, bool visible);
	void SetCategoryFilter(const std::string& category, bool visible);

	bool IsThreadVisible(const std::string& name) const;
	bool IsCategoryVisible(const std::string& category) const;

	void SetPauseThreshold(uint64 threshold) { _pauseThreshold = threshold; }	
	uint64 GetPauseThreshold() const { return _pauseThreshold; }	
	
	bool IsRunning() const { return _isRunning; }	
	bool IsPaused() const { return _isPaused; }

private:
	int32	GetThreadIndex(uint32 threadID);
	void	SetProfileEventColor(ProfileEvent& event);	

private:
	bool _isRunning = false;
	bool _isPaused = false;	

	uint64 _startTimestamp = 0;
	uint64 _pauseTimestamp = 0;
	uint64 _pauseThreshold = 100000;	// 100ms ±âº»°ª
	
	uint32 _currentFrameIndex = 0;	
	uint32 _maxDepth = 0;	

	std::vector<ThreadInfo> _threadsInfo;
	std::vector<FrameInfo> _framesInfo;	

	std::unordered_map<uint32, std::vector<ProfileEvent>> _activeEvents;	
	std::unordered_map<std::string, bool> _threadFilters;
	std::unordered_map<std::string, bool> _categoryFilters;	
	std::unordered_map<std::string, std::array<float, 4>> _categoryColors;

	std::mutex _lock;
};

class ScopedProfiler
{
public:
	ScopedProfiler(const std::string& name, const std::string& category = "Default")
		: _name(name), _category(category)
	{
		Profiler::GetInstance().BeginEvent(_name, _category);
	}
	~ScopedProfiler()
	{
		Profiler::GetInstance().EndEvent(_name);
	}

private:
	std::string _name;
	std::string _category;
};

#define PROFILE_FUNCTION() ScopedProfiler _scopedProfiler(__FUNCTION__, "Functions")
#define PROFILE_SCOPE(name, category) ScopedProfiler _scopedProfiler(name, category)
#define PROFILE_THREAD(name) Profiler::GetInstance().SetThreadName(std::hash<std::thread::id>{}(std::this_thread::get_id()), name)
#define PROFILE_BEGIN_FRAME(name) Profiler::GetInstance().BeginFrame(name)
#define PROFILE_END_FRAME() Profiler::GetInstance().EndFrame()