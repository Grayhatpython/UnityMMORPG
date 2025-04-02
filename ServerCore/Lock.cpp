#include "pch.h"
#include "Lock.h"

void Lock::WriteLock()
{
	//	현재 WriteLock을 들고 있는 ThreadId 확인
	auto threadId = _ownerThreadId.load();
	//	스핀 카운트 초기화
	LThreadSpinCount.spinCount = INITIAL_SPIN_COUNT;

	//	현재 WriteLock을 들고 있는 ThreadId와 WriteLock을 시도하는 ThreadId가 같은 경우
	//	중첩 Lock 카운트 증가
	if (threadId == LThreadId)
	{
		_nestedCount.fetch_add(1);
		return;
	}

	//	다른 Thread가 WriteLock을 들고 있는 경우
	while (true)
	{
		uint32 expected = EMPTY_OWNER_THREAD_ID;
		if (_ownerThreadId.compare_exchange_strong(OUT expected, LThreadId))
		{
			//	현재 호출한 Thread가 WriteLock
			::AcquireSRWLockExclusive(&_lock);
			//	호출한 ThreadId를 ownerThreadId에 저장
			_ownerThreadId.store(LThreadId);
			//	중첩 Lock 카운트 증가
			_nestedCount.fetch_add(1);
			return;
		}

		//	스레드별 스핀 카운트 사용
		uint32 spinCount = LThreadSpinCount.spinCount;
		for (uint32 i = 0; i < spinCount; i++)
		{
			YieldProcessor();
		}

		//	스핀 카운트를 점진적으로 증가
		if (spinCount < MAX_SPIN_COUNT)
		{
			LThreadSpinCount.spinCount = spinCount * 2;
		}
	}
}

void Lock::WriteUnLock()
{
	//	현재 WriteLock을 들고 있는 ThreadId 확인
	auto threadId = _ownerThreadId.load();

	//	현재 WriteLock을 들고 있는 ThreadId와 WriteLock을 시도하는 ThreadId가 같은 경우
	//	중첩 Lock 카운트 감소
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