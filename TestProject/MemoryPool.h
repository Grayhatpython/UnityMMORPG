#pragma once

struct alignas(16) MemoryBlockHeader
{
	MemoryBlockHeader(size_t allocSize, size_t dataSize)
		: allocSize(allocSize), dataSize(dataSize)
	{
	}


	static void* GetDataPos(MemoryBlockHeader* header)
	{
		return reinterpret_cast<void*>(++header);
	}

	static MemoryBlockHeader* GetHeaderPos(void* memory)
	{
		return reinterpret_cast<MemoryBlockHeader*>(memory) - 1;
	}

	size_t		allocSize = 0;
	size_t		dataSize = 0;
	//	Padding
};


class ThreadLocalCache
{
	friend class GlobalMemoryPool;

	static constexpr size_t S_CACHE_BUCKET_SIZE = 32;
	static constexpr size_t S_MAX_CACHE_BLOCK_SIZE = 50;

	struct Bucket
	{
		void* blocks[S_MAX_CACHE_BLOCK_SIZE];
		size_t count = 0;
	};

public:
	ThreadLocalCache()
	{

	}

	~ThreadLocalCache()
	{

	}

public:
	void* Allocate(size_t size)
	{
		const auto bucketIndex = GetBucketIndexFromSize(size);
		auto& bucket = _buckets[bucketIndex];

		if (bucket.count > 0)
		{
			void* memory = bucket.blocks[--bucket.count];
			return memory;
		}

		return nullptr;
	}

	bool Deallocate(MemoryBlockHeader* memoryBlockHeader)
	{
		const auto bucketIndex = GetBucketIndexFromSize(memoryBlockHeader->dataSize);
		auto& bucket = _buckets[bucketIndex];

		if (bucket.count < S_MAX_CACHE_BLOCK_SIZE)
		{
			bucket.blocks[bucket.count++] = memoryBlockHeader;
			return true;
		}

		return false;
	}

	void Refill(size_t bucketIndex, size_t size, size_t count);

private:
	static size_t GetBucketIndexFromSize(size_t size)
	{
		size_t extractionNumber = std::min(std::max(size, static_cast<size_t>(32)), static_cast<size_t>(4096));

		if (extractionNumber <= 256) {
			// 32~256 범위는 32바이트 단위로 분류
			return (extractionNumber - 1) / 32;
		}
		else if (extractionNumber <= 1024) {
			// 256~1024 범위는 128바이트 단위로 분류
			return 8 + (extractionNumber - 256 - 1) / 128;
		}
		else {
			// 1024~4096 범위는 512바이트 단위로 분류
			return 14 + (extractionNumber - 1024 - 1) / 512;
		}
	}

	static size_t GetSizeFromBucketIndex(size_t index)
	{
		if (index < 8) {
			// 32~256 범위
			return (index + 1) * 32;
		}
		else if (index < 14) {
			// 256~1024 범위
			return 256 + (index - 7) * 128;
		}
		else {
			// 1024~4096 범위
			return 1024 + (index - 13) * 512;
		}
	}

private:
	std::array<Bucket, S_CACHE_BUCKET_SIZE> _buckets;
};

class GlobalMemoryPool
{
	static constexpr size_t S_MAX_FREELIST_COUNT = 32;
	static constexpr size_t S_MAX_BLOCK_SIZE = 4096;
	static constexpr size_t S_MIN_BLOCK_SIZE = 32;

	struct FreeList
	{
		std::vector<void*> lists;
	};

public:
	// 메모리 할당
	void* Allocate(size_t size) {
		void* memory = AllocateInternal(size);
		return memory;
	}

	// 메모리 해제
	void Deallocate(void* ptr) 
	{
		if (ptr) 
			DeallocateInternal(ptr);
	}

	// 객체 생성과 동시에 할당
	template <typename T, typename... Args>
	T* New(Args&&... args) 
	{
		void* memory = AllocateInternal(sizeof(T));
		return new(memory) T(std::forward<Args>(args)...);
	}

	// 객체 소멸 후 메모리 해제
	template <typename T>
	void Delete(T* ptr) 
	{
		if (ptr) {
			ptr->~T();
			DeallocateInternal(static_cast<void*>(ptr));
		}
	}

	// 스마트 포인터 생성
	template <typename T, typename... Args>
	std::shared_ptr<T> MakeShared(Args&&... args) {
		T* ptr = New<T>(std::forward<Args>(args)...);
		return std::shared_ptr<T>(ptr, [this](T* p) { Delete(p); });
	}


	static GlobalMemoryPool& Instance() {
		static GlobalMemoryPool instance;
		return instance;
	}

	static ThreadLocalCache& GetThreadLocalCache()
	{
		thread_local ThreadLocalCache LThreadLocalCache;
		return LThreadLocalCache;
	}

	static size_t GetBucketIndexFromThreadLocalCache(size_t size)
	{
		return ThreadLocalCache::GetBucketIndexFromSize(size);
	}
	
	static void ThreadLocalCacheClear()
	{
		auto& threadLocalCache = GetThreadLocalCache();
		for (size_t i = 0; i < ThreadLocalCache::S_CACHE_BUCKET_SIZE; i++)
		{
			auto& bucket = threadLocalCache._buckets[i];
			for (size_t j = 0; j < bucket.count; j++)
			{
				if (bucket.blocks[j])
				{
					::_aligned_free(bucket.blocks[j]);
					bucket.blocks[j] = nullptr;
				}
			}

			bucket.count = 0;
		}

	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(_lock);

		for (auto& freeList : _freeLists)
		{
			for (auto memory : freeList.lists)
			{
				if(memory)
					::_aligned_free(memory);
			}

			freeList.lists.clear();
		}
	}

public:

	void* AllocateFromGlobalMemoryPool(size_t size)
	{
		auto bucketIndex = GetBucketIndexFromThreadLocalCache(size);

		{
			std::lock_guard<std::mutex> lock(_lock);

			FreeList& freeList = _freeLists[bucketIndex];

			if (freeList.lists.empty() == false)
			{
				auto memoryBlockHeader = static_cast<MemoryBlockHeader*>(freeList.lists.back());
				freeList.lists.pop_back();
				return memoryBlockHeader;
			}
		}

		return AllocateNewMemory(size);
	}

	void DeallocateToGlobalMemoryPool(void* memory)
	{
		auto memoryBlockHeader = static_cast<MemoryBlockHeader*>(memory);
		auto bucketIndex = GetBucketIndexFromThreadLocalCache(memoryBlockHeader->dataSize);

		std::lock_guard<std::mutex> lock(_lock);

		FreeList& freeList = _freeLists[bucketIndex];
		freeList.lists.push_back(memoryBlockHeader);
	}

private:
	void* AllocateInternal(size_t dataSize)
	{
		auto& threadLocalCache = GetThreadLocalCache();
		void* memory = threadLocalCache.Allocate(dataSize);

		if (memory == nullptr)
		{
			auto bucketIndex = GetBucketIndexFromThreadLocalCache(dataSize);
			threadLocalCache.Refill(bucketIndex, dataSize, 25);

			memory = threadLocalCache.Allocate(dataSize);

			if (memory == nullptr)
				memory = AllocateNewMemory(dataSize);

		}

		return MemoryBlockHeader::GetDataPos(reinterpret_cast<MemoryBlockHeader*>(memory));
	}

	void DeallocateInternal(void* memory)
	{
		if (memory == nullptr)
			return;

		auto& threadLocalCache = GetThreadLocalCache();

		MemoryBlockHeader* memoryBlockHeader = MemoryBlockHeader::GetHeaderPos(memory);

		if (threadLocalCache.Deallocate(memoryBlockHeader) == false)
			DeallocateToGlobalMemoryPool(memoryBlockHeader);
	}


	void* AllocateNewMemory(size_t dataSize)
	{
		size_t totalSize = dataSize + sizeof(MemoryBlockHeader);
		void* memory = ::_aligned_malloc(totalSize, 16);

		//	TODO
		new(memory)MemoryBlockHeader(totalSize, dataSize); // placement new

		return memory;
	}

private:
	std::array<FreeList, S_MAX_FREELIST_COUNT> _freeLists;
	std::mutex _lock;
};

// 전역 함수: 메모리 해제
template <typename T>
void cdelete(T* ptr) {
	GMemoryPool.Delete(ptr);
}

// 전역 함수: 객체 생성
template <typename T, typename... Args>
T* cnew(Args&&... args) {
	return GMemoryPool.New<T>(std::forward<Args>(args)...);
}

// 전역 함수: std::shared_ptr 생성
template <typename T, typename... Args>
std::shared_ptr<T> MakeShared(Args&&... args) {
	return GMemoryPool.MakeShared<T>(std::forward<Args>(args)...);
}