#pragma once

#include <thread>
#include <functional>

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void	Launch(function<void(void)> callback, const std::wstring& threadName = L"");
	void	Join();

	static void InitTLS();
	static void DestroyTLS();

	static void DoGlobalJobQueueExecute();
	static void DistributeReserveJobs();

public:
	void			Close();

private:
	Mutex			_lock;
	vector<thread>	_threads;
	Atomic<bool>	_stopped = false;
};

