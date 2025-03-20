#include "pch.h"
#include "JobQueueBasedObject.h"
#include "GlobalQueue.h"

void JobQueueBasedObject::Push(JobRef job, bool isPushOnly)
{
	const auto prevCount = _jobCount.fetch_add(1);
	//	job : �ް�
	_jobQueue.Push(job);

	if(isPushOnly == false)
	{
		//	ù��°�� �ϰ��� Push�� �����尡 �������
		if (prevCount == 0)
		{
			//	�����尡 �ϰ��� ó���ϰ� �ִ� JobQueue�� ���� ��� �� JobQueue�� �ϰ��� ���� ó���Ѵ�.
			if (LCurrentExecuteJobQueue == nullptr)
			{
				ScheduleTimeExecute();
			}
			//	�����尡 �ϰ��� ó���ϰ� �ִ� JobQueue�� ���� ��� GlobalQueue�� 
			//	�ٸ� �����ִ� �����忡�� JobQueue�� ó���϶�� Push�Ѵ�.
			else
			{
				GGlobalQueue->Push(shared_from_this());
			}
		}
	}
}

void JobQueueBasedObject::ScheduleTimeExecute()
{
	//	�����尡 �� JobQueue�� �ϰ��� ó���ϰ� �ִ�.
	LCurrentExecuteJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobQueue.PopAll(OUT jobs);

		const auto jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute();

		//	���� ó���ϰ� �ִ� ���� job Push�� ���� ��� �������´�.
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			//	�����尡 �� JobQueue�� �ϰ��� ó���ϰ� ���� �ִ� �ϰ��� ����.
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