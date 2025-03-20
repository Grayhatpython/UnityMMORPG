#pragma once

struct JobData
{
	JobData(weak_ptr<JobQueueBasedObject> owner, JobRef job)
		: owner(owner), job(job)
	{

	}

	//	������ ���� ��ü�� ���� �ð��� �Ǿ��� �� �Ҹ�Ǿ��� ���� �����Ƿ� weak_ptr
	weak_ptr<JobQueueBasedObject>	owner;
	JobRef							job;
};

struct TimerItem
{
	bool operator<(const TimerItem& other) const
	{
		return executeTick > other.executeTick;
	}

	uint64		executeTick = 0;
	//	PQ �˰��� Ư���� ���糪 ����Ʈ�����ͻ��� Sort ������� �߻�
	JobData*	jobData = nullptr;
};

class JobTimer
{
public:
	void Reserve(uint64 tickAfter, weak_ptr<JobQueueBasedObject> owner, JobRef job);
	void Distribute(uint64 now);
	void Clear();

private:
	USE_LOCK;
	PriorityQueue<TimerItem>	_items;
	Atomic<bool>				_isDistribute = false;
};

