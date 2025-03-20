#include "pch.h"
#include "MemoryPool.h"


MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize)
{
	::InitializeSListHead(&_header);
}

MemoryPool::~MemoryPool()
{
	while (MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header)))
		::_aligned_free(memory);
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	ptr->allocSize = 0;

	::InterlockedPushEntrySList(&_header, static_cast<PSLIST_ENTRY>(ptr));

	_useCount.fetch_sub(1);
	_reserveCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header));

	// 없으면 새로 만들다
	if (memory == nullptr)
	{
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(_allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		ASSERT_CRASH(memory->allocSize == 0);
		_reserveCount.fetch_sub(1);
	}

	_useCount.fetch_add(1);

	return memory;
}

Memory::Memory()
{
	int32 size = 0;
	int32 tableIndex = 0;

	for (size = 32; size < 1024; size += 32)
	{
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		for(int i = tableIndex +1; i <= size; i++)
			_poolTable[i] = pool;

		tableIndex = size;
	}

	for (size = 1024; size <= 2048; size += 128)
	{
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		for (int i = tableIndex + 1; i <= size; i++)
			_poolTable[i] = pool;

		tableIndex = size;
	}

	for (size = 2048; size <= 4096; size += 256)
	{
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		for (int i = tableIndex + 1; i <= size; i++)
			_poolTable[i] = pool;

		tableIndex = size;
	}
}

Memory::~Memory()
{
	for (MemoryPool* pool : _pools)
		delete pool;

	_pools.clear();
}

void* Memory::Allocate(int32 size)
{
	MemoryHeader* header = nullptr;
	const int32 memorySize = size + sizeof(MemoryHeader);

	if (memorySize > MAX_ALLOC_SIZE)
	{
		// 메모리 풀링 최대 크기를 벗어나면 일반 할당
		header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(memorySize, SLIST_ALIGNMENT));
	}
	else
	{
		const int32 allocSize = _poolTable[memorySize]->GetAllocSize();

		if (LThreadCacheMemory)
		{
			if (LThreadCacheMemory->IsEmptyMemoryBlocks(allocSize))
			{
				for (int i = 0; i < ThreadCacheMemory::MEMORY_BLOCK_REQUEST_CHUNK_SIZE; i++)
				{
					// 메모리 풀에서 꺼내온다
					header = _poolTable[allocSize]->Pop();
					LThreadCacheMemory->Push(allocSize, header);
				}
			}

			header = LThreadCacheMemory->Pop(allocSize);
		}
		else
			// 메모리 풀에서 꺼내온다
			header = _poolTable[allocSize]->Pop();
	}

	return MemoryHeader::AttachHeader(header, memorySize);
}

void Memory::Release(void* ptr)
{
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);

	const int32 memorySize = header->allocSize;
	ASSERT_CRASH(memorySize > 0);

	if (memorySize > MAX_ALLOC_SIZE)
	{
		// 메모리 풀링 최대 크기를 벗어나면 일반 해제
		::_aligned_free(header);
	}
	else
	{
 		const int32 allocSize = _poolTable[memorySize]->GetAllocSize();

		if(LThreadCacheMemory)
			LThreadCacheMemory->Push(allocSize, header);
		else
		//	 메모리 풀에 반납한다
			_poolTable[allocSize]->Push(header);
	}
}


ThreadCacheMemory::~ThreadCacheMemory()
{
	for (auto& memoryBlock : _memoryBlocks)
	{
		for (auto memory : memoryBlock.second)
		{
			::_aligned_free(memory);
		}
	}
}

void ThreadCacheMemory::Push(int32 allocSize, MemoryHeader* memoryBlock)
{
	_memoryBlocks[allocSize].push_back(memoryBlock);
}

MemoryHeader* ThreadCacheMemory::Pop(int32 allocSize)
{
	auto& memoryBlocks = _memoryBlocks[allocSize];
	MemoryHeader* memoryBlock = memoryBlocks.back();
	memoryBlocks.pop_back();
	return memoryBlock;
}

bool ThreadCacheMemory::IsEmptyMemoryBlocks(int32 allocSize)
{
	auto find = _memoryBlocks.find(allocSize);
	if (find == _memoryBlocks.end())
		return true;

	return find->second.empty();
}
