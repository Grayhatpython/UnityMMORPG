#pragma once

enum
{
	SLIST_ALIGNMENT = 16
};

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
	// [MemoryHeader][Data]
	MemoryHeader(int32 size) : allocSize(size) { }

	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		new(header)MemoryHeader(size); // placement new
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	//	실제 크기랑 할당된 크기랑 다를 수 있다.
	int32 allocSize = 0;
	// TODO : 필요한 추가 정보
};


DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void			Push(MemoryHeader* ptr);
	MemoryHeader*	Pop();

public:
	int32			GetAllocSize() const { return _allocSize; }

private:
	SLIST_HEADER	_header;
	int32			_allocSize = 0;
	atomic<int32>	_useCount = 0;
	atomic<int32>	_reserveCount = 0;
};

class Memory
{
	enum
	{
		// ~1024까지 32단위, ~2048까지 128단위, ~4096까지 256단위
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void*	Allocate(int32 size);
	void	Release(void* ptr);

private:
	vector<MemoryPool*> _pools;
	MemoryPool*			_poolTable[MAX_ALLOC_SIZE + 1];
};

class ThreadCacheMemory
{
public:
	enum
	{
		MEMORY_BLOCK_REQUEST_CHUNK_SIZE = 10
	};

public:
	~ThreadCacheMemory();

	void			Push(int32 allocSize, MemoryHeader* memoryBlock);
	MemoryHeader*	Pop(int32 allocSize);


public:
	bool			IsEmptyMemoryBlocks(int32 allocSize);

private:
	//	TEMP
	std::map<int32, std::vector<MemoryHeader*>>		_memoryBlocks;
};

template<typename Type, typename... Args>
Type* cnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(Allocator::Allocate(sizeof(Type)));
	new(memory)Type(std::forward<Args>(args)...); // placement new
	return memory;
}

template<typename Type>
void cdelete(Type* obj)
{
	obj->~Type();
	Allocator::Deallocate(obj);
}

template<typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args&&... args)
{
	return shared_ptr<Type>{ cnew<Type>(std::forward<Args>(args)...), cdelete<Type> };
}