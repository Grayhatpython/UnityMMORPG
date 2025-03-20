#include "pch.h"
#include "JobQueueBasedObject.h"
#include "GlobalQueue.h"

void JobQueueBasedObject::Push(JobRef job, bool isPushOnly)
{
	const auto prevCount = _jobCount.fetch_add(1);
	//	job : 왼값
	_jobQueue.Push(job);

	if(isPushOnly == false)
	{
		//	첫번째로 일감을 Push한 스레드가 실행까지
		if (prevCount == 0)
		{
			//	스레드가 일감을 처리하고 있는 JobQueue가 없을 경우 이 JobQueue의 일감을 내가 처리한다.
			if (LCurrentExecuteJobQueue == nullptr)
			{
				ScheduleTimeExecute();
			}
			//	스레드가 일감을 처리하고 있는 JobQueue가 있을 경우 GlobalQueue에 
			//	다른 여유있는 스레드에서 JobQueue를 처리하라고 Push한다.
			else
			{
				GGlobalQueue->Push(shared_from_this());
			}
		}
	}
}

void JobQueueBasedObject::ScheduleTimeExecute()
{
	//	스레드가 이 JobQueue의 일감을 처리하고 있다.
	LCurrentExecuteJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobQueue.PopAll(OUT jobs);

		const auto jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute();

		//	내가 처리하고 있는 동안 job Push가 없는 경우 빠져나온다.
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			//	스레드가 이 JobQueue의 일감을 처리하고 남아 있는 일감이 없다.
			LCurrentExecuteJobQueue = nullptr;
			break;
		}

		const uint64 now = ::GetTickCount64();

		if (now >= LEndTickCount)
		{
			LCurrentExecuteJobQueue = nullptr;
			GGlobalQueue->Push(shared_from_this());
			break;
		}
	}
}

void JobQueueBasedObject::PopAllExecute()
{
	Vector<JobRef> jobs;
	_jobQueue.PopAll(jobs);

	for (auto i = 0; i < jobs.size(); i++)
		jobs[i]->Execute();

	LCurrentExecuteJobQueue = nullptr;
}