#pragma once

struct JobData
{
	JobData(weak_ptr<JobQueueBasedObject> owner, JobRef job)
		: owner(owner), job(job)
	{

	}

	//	예약해 놓은 주체가 예약 시간이 되었을 떄 소멸되었을 수도 있으므로 weak_ptr
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
	//	PQ 알고리즘 특성상 복사나 스마트포인터사용시 Sort 오버헤드 발생
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

