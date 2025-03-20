#include "pch.h"
#include "Lock.h"

void Lock::WriteLock()
{
	//	현재 WriteLock을 잡고 있는 ThreadId를 확인
	auto threadId = _ownerThreadId.load();

	//	현재 WriteLock을 잡고 있는 ThreadId와 지금 WriteLock을 하는 ThreadId가 같으면 통과
	//	중첩 Lock 카운트 증가
	if (threadId == LThreadId)
	{
		_nestedCount.fetch_add(1);
		return;
	}

	//	지금 호출한 Thread가 아닌 다른 Thread가 WriteLock을 걸고 있는 경우
	while (true)
	{
		uint32 expected = EMPTY_OWNER_THREAD_ID;
		if (_ownerThreadId.compare_exchange_strong(OUT expected, LThreadId))
		{
			//	지금 호출한 Thread가 WriteLock
			::AcquireSRWLockExclusive(&_lock);
			//	호출한 ThreadId를 ownerThreadId에 저장
			_ownerThreadId.store(LThreadId);
			//	중첩 Lock 카운트 증가
			_nestedCount.fetch_add(1);
			return;
		}

		for (auto i = 0; i < LOCK_SPIN_COUNT; i++)
		{
			YieldProcessor();
			//std::this_thread::yield();
		}
	}
}

void Lock::WriteUnLock()
{
	//	현재 WriteLock을 잡고 있는 ThreadId를 확인
	auto threadId = _ownerThreadId.load();

	//	현재 WriteLock을 잡고 있는 ThreadId와 지금 WriteLock을 하는 ThreadId가 같으면 통과
	//	중첩 Lock 카운트 증가
	if (threadId == LThreadId)
	{
		auto prevCount = _nestedCount.fetch_sub(1);

		if (prevCount == 1)
		{
			::ReleaseSRWLockExclusive(&_lock);
			_ownerThreadId.store(EMPTY_OWNER_THREAD_ID);
		}
	}
}