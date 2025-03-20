#include "pch.h"
#include "ThreadManager.h"
#include "CoreGlobal.h"
#include "GlobalQueue.h"
#include "Log.h"

ThreadManager::ThreadManager()
{
	// Main Thread
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback, const std::wstring& threadName)
{
	LockGuard guard(_lock);

	_threads.push_back(thread([=]()
		{
			InitTLS();
			
			if(threadName.empty() == false)
				::SetThreadDescription(::GetCurrentThread(), threadName.c_str());
			
			while(_stopped == false)
				callback();

			DestroyTLS();
		}));
}


void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}
	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);

	LThreadCacheMemory = new ThreadCacheMemory();
	LThreadCallStackInfo = new std::vector<ThreadCallStackInfo>();
}

void ThreadManager::DestroyTLS()
{
	if (LThreadCallStackInfo)
	{
		delete LThreadCallStackInfo;
		LThreadCallStackInfo = nullptr;
	}
	if (LThreadCacheMemory)
	{
		delete LThreadCacheMemory;
		LThreadCacheMemory = nullptr;
	}
}

void ThreadManager::DoGlobalJobQueueExecute()
{
	while (true)
	{
		uint64 now = ::GetTickCount64();

		//	��� ���� �ð����� �ϰ��� ó������
		if (now > LEndTickCount)
			break;

		//	GlobalQueue���� ó�� ����ϰ� �ִ� JobQueue�� ���´�.
		auto jobQueue = GGlobalQueue->Pop();
		if (jobQueue == nullptr)
			break;

		//	�� ������� JobQueue�� ó���Ѵ�.
		jobQueue->ScheduleTimeExecute();
	}
}

//	����� Job�� ���
void ThreadManager::DistributeReserveJobs()
{
	const uint64 now = ::GetTickCount64();

	GJobTimer->Distribute(now);
}

void ThreadManager::Close()
{
	_stopped.store(true); 
	DestroyTLS();
}
