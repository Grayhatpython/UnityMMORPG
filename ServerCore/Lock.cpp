#include "pch.h"
#include "Lock.h"

void Lock::WriteLock()
{
	//	���� WriteLock�� ��� �ִ� ThreadId�� Ȯ��
	auto threadId = _ownerThreadId.load();

	//	���� WriteLock�� ��� �ִ� ThreadId�� ���� WriteLock�� �ϴ� ThreadId�� ������ ���
	//	��ø Lock ī��Ʈ ����
	if (threadId == LThreadId)
	{
		_nestedCount.fetch_add(1);
		return;
	}

	//	���� ȣ���� Thread�� �ƴ� �ٸ� Thread�� WriteLock�� �ɰ� �ִ� ���
	while (true)
	{
		uint32 expected = EMPTY_OWNER_THREAD_ID;
		if (_ownerThreadId.compare_exchange_strong(OUT expected, LThreadId))
		{
			//	���� ȣ���� Thread�� WriteLock
			::AcquireSRWLockExclusive(&_lock);
			//	ȣ���� ThreadId�� ownerThreadId�� ����
			_ownerThreadId.store(LThreadId);
			//	��ø Lock ī��Ʈ ����
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
	//	���� WriteLock�� ��� �ִ� ThreadId�� Ȯ��
	auto threadId = _ownerThreadId.load();

	//	���� WriteLock�� ��� �ִ� ThreadId�� ���� WriteLock�� �ϴ� ThreadId�� ������ ���
	//	��ø Lock ī��Ʈ ����
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