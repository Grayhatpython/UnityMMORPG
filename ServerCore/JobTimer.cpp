#include "pch.h"
#include "JobTimer.h"
#include "JobQueueBasedObject.h"

void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueueBasedObject> owner, JobRef job)
{
	const uint64 executeTick = ::GetTickCount64() + tickAfter;
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

	WRITE_LOCK;
	_items.push(TimerItem{ executeTick, jobData });
}

void JobTimer::Distribute(uint64 now)
{
	//	One thread
	if (_isDistribute.exchange(true) == true)
		return;

	Vector<TimerItem> items;
	{
		WRITE_LOCK;

		while (_items.empty() == false)
		{
			const TimerItem& timerItem = _items.top();
			if (now < timerItem.executeTick)
				break;

			items.push_back(timerItem);
			_items.pop();
		}
	}

	for (auto& item : items)
	{
		if (JobQueueBasedObjectRef owner = item.jobData->owner.lock())
		{
			owner->Push(item.jobData->job, true);
		}

		ObjectPool<JobData>::Push(item.jobData);
	}

	_isDistribute.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;

	while (_items.empty() == false)
	{
		const TimerItem& timerItem = _items.top();
		ObjectPool<JobData>::Push(timerItem.jobData);
		_items.pop();
	}
}
