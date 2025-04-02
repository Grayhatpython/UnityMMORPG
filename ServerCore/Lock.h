#pragma once
#include "Types.h"

struct ThreadSpinCount
{
	uint32 spinCount = 0;
	uint32 maxSpinCount = 1024;
};

class Lock
{
	enum
	{
		EMPTY_OWNER_THREAD_ID = 0,
		INITIAL_SPIN_COUNT = 64,
		MAX_SPIN_COUNT = 1024,
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
