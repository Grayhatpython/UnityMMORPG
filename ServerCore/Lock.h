#pragma once
#include "Types.h"

class Lock
{
	enum
	{
		EMPTY_OWNER_THREAD_ID = 0,
		LOCK_SPIN_COUNT = 1024,
	};

public:
	Lock()
	{
		::InitializeSRWLock(&_lock);
	}

public:
	void WriteLock();
	void WriteUnLock();

private:
	SRWLOCK			_lock;
	atomic<uint32>	_ownerThreadId = 0;
	atomic<uint32>	_nestedCount = 0;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock)
		: _lock(lock)
	{
		_lock.WriteLock();
	}
	~WriteLockGuard()
	{
		_lock.WriteUnLock();
	}

private:
	Lock& _lock;
};
