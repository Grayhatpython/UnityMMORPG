#pragma once
#include "Job.h"
#include "LockQueue.h"
#include "JobTimer.h"

class JobQueueBasedObject : public std::enable_shared_from_this<JobQueueBasedObject>
{
public:
	void PushJob(CallbackType&& callback, bool isPushOnly = false)
	{
		auto job = ObjectPool<Job>::MakeShared(std::move(callback));
		Push(std::move(job), isPushOnly);
	}

	template<typename T, typename Ret, typename... Args>
	void PushJob(Ret(T::*memFunc)(Args...), bool isPushOnly = false, Args... args)
	{
		//	Ref Count 증가 -> Cycle Check
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		auto job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		Push(std::move(job), isPushOnly);
	}

	void PushReserveJob(uint64 tickAfter, CallbackType&& callback)
	{
		auto job = ObjectPool<Job>::MakeShared(std::move(callback));
		//	Timer는 weak_ptr Ref Count x
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	template<typename T, typename Ret, typename... Args>
	void PushReserveJob(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		//	Ref Count 증가 -> Cycle Check
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		auto job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	void Clear() { _jobQueue.Clear(); }

public:
	//	move
	void	Push(JobRef job, bool isPushOnly = false);
	void	ScheduleTimeExecute();
	void	PopAllExecute();

protected:
	LockQueue<JobRef>	_jobQueue;
	Atomic<int32>		_jobCount = 0;
};

