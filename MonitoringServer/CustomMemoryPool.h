#pragma once

#include <atomic>
#include <vector>
#include <array>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <cassert>
#include <type_traits>
#include <cstdint>
#include <algorithm>
#include <random>

//	캐시 라인 매크로
constexpr size_t CACHE_LINE_SIZE = 64;
#define CASH_LINE_ALIGNED alignas(CACHE_LINE_SIZE)

//	메모리 블럭 헤더
struct CASH_LINE_ALIGNED MemoryBlockHeader
{
	MemoryBlockHeader(size_t size, size_t index)
		:allocSize(size), poolIndex(index)
	{

	}

	static void* AttachHeader(MemoryBlockHeader* header)
	{
		return reinterpret_cast<void*>(++header);
	}

	static MemoryBlockHeader* DetachHeader(void* ptr)
	{
		MemoryBlockHeader* header = reinterpret_cast<MemoryBlockHeader*>(ptr) - 1;
		return header;
	}

	size_t allocSize = 0;
	size_t poolIndex = 0;
	uint64 timestamp = 0;
};

struct CASH_LINE_ALIGNED AlignedMutex
{
	std::mutex mutex;
};

template<size_t N>
class AlignedMutexArray
{
public:
	std::mutex& GetMutex(size_t index)
	{
		return _mutexs[index & N].mutex;
	}

private:
	AlignedMutex _mutexs[N];
};

template<typename T>
class CASH_LINE_ALIGNED ShardedPool
{
private:

	struct Node
	{ 
		Node(const T& value)
		: data(value), next(nullptr)
		{

		}

		T		data;
		Node*	next = nullptr;
	};

	struct CASH_LINE_ALIGNED Pool
	{
		Pool()
			: head(nullptr)
		{

		}

		Node* head = nullptr;
		std::mutex mutex;
	};

public:
	~ShardedPool()
	{
		for (auto& shard : _shards)
		{
			std::lock_guard<std::mutex> lock(shard.mutex);
			while (shard.head)
			{
				Node* next = shard.head->next;
				delete shard.head;
				shard.head = next;
			}
		}
	}

public:
	void Push(const T& value)
	{
		size_t shardIndex = GetShardIndex(true);
		auto& shard = _shards[shardIndex];

		Node* newNode = new Node(value);

		{
			std::lock_guard<std::mutex> lock(shard.mutex);
			newNode->next = shard.head;
			shard.head = newNode;
		}
	}

	bool Pop(T out)
	{
		size_t shardIndex = GetShardIndex(false);

		{
			auto& shard = _shards[shardIndex];
			std::lock_guard<std::mutex> lock(shard.mutex);
			if (shard.head)
			{
				out = shard.head->data;
				Node* oldHead = shard.head;
				shard.head = shard.head->next;
				delete oldHead;
				return true;
			}
		}

		{
			for (size_t i = 0; i < S_SHARED_COUNT; i++) {
				// 선호 샤드는 이미 시도했으므로 건너뛰기
				if (i == shardIndex) continue;

				auto& shard = _shards[i];
				std::lock_guard<std::mutex> lock(shard.mutex);
				if (shard.head) {
					out = shard.head->data;
					Node* oldHead = shard.head;
					shard.head = shard.head->next;
					delete oldHead;
					return true;
				}
			}
		}

		return false;
	}

public:
	static size_t GetRandomShardIndex()
	{
		std::uniform_int_distribution<size_t> dist(0, S_SHARED_COUNT - 1);
		return dist(S_LThreadRandomShardGenerate);
	}

	size_t GetShardIndex(bool preferred)
	{
		if (S_LThreadPreferredShardIndex == -1)
			S_LThreadPreferredShardIndex = GetRandomShardIndex();

		if (preferred)
			return S_LThreadPreferredShardIndex;
		else
			return GetRandomShardIndex();
	}

private:
	static constexpr size_t S_SHARED_COUNT = 16;
	//	각 스레드는 해당 클래스에 대해 하나의 static 변수 인스턴스를 사용
	static thread_local	std::mt19937 S_LThreadRandomShardGenerate;
	static thread_local size_t S_LThreadPreferredShardIndex;

	Pool _shards[S_SHARED_COUNT];
};

template<typename T>
thread_local std::mt19937 ShardedPool<T>::S_LThreadRandomShardGenerate(std::random_device{}());

template<typename T>
thread_local size_t ShardedPool<T>::S_LThreadPreferredShardIndex = -1;


class CASH_LINE_ALIGNED PoolAllocator
{
private:
	struct MemoryChunk
	{
		MemoryChunk(size_t size)
			: size(size), used(0), next(nullptr)
		{
			memory = ::malloc(size);
		}

		~MemoryChunk()
		{
			if (memory)
				::free(memory);
		}

		size_t GetPaddingSize(size_t alignSize)
		{
			size_t paddingSize = (alignSize - (reinterpret_cast<uintptr_t>(static_cast<char*>(memory) + used) % alignSize)) % alignSize;
			return paddingSize;
		}

		bool CanAllocate(size_t allocSize, size_t alignSize)
		{
			auto paddingSize = GetPaddingSize(alignSize);
			return (used + paddingSize + allocSize <= size);
		}

		void* Allocate(size_t allocSize, size_t alignSize)
		{
			auto paddingSize = GetPaddingSize(alignSize);
			void* result = static_cast<char*>(memory) + used + paddingSize;
			used += paddingSize + allocSize;
			return result;
		}

		MemoryChunk* next = nullptr;
		void*	memory = nullptr;
		size_t	size = 0;
		size_t	used = 0;
	};

public:
	static void* Allocate(size_t allocSize, size_t alignSize = alignof(std::max_align_t))
	{
		if (allocSize == 0)
			return nullptr;

		if (S_LThreadCurrentChunk == nullptr)
		{
			if (S_LThreadChunkSize == 0)
				S_LThreadChunkSize = S_DEFAULT_CHUNK_SIZE;

			S_LThreadCurrentChunk = new MemoryChunk(S_DEFAULT_CHUNK_SIZE);
			S_LThreadLocalChunks = S_LThreadCurrentChunk;
		}

		if (allocSize > S_LThreadChunkSize / 4)
			return ::_aligned_malloc(allocSize, alignSize);

		if (S_LThreadCurrentChunk->CanAllocate(allocSize, alignSize) == true)
			return S_LThreadCurrentChunk->Allocate(allocSize, alignSize);

		MemoryChunk* newChunk = new MemoryChunk(S_LThreadChunkSize);
		newChunk->next = S_LThreadLocalChunks;
		S_LThreadLocalChunks = newChunk;
		S_LThreadCurrentChunk = newChunk;

		return S_LThreadCurrentChunk->Allocate(allocSize, alignSize);
	}

public:
	static void SetChunkSize(size_t size)
	{
		S_LThreadChunkSize = size;
	}

private:
	static constexpr size_t S_DEFAULT_CHUNK_SIZE = 1024 * 1024;

	static thread_local MemoryChunk* S_LThreadLocalChunks;
	static thread_local MemoryChunk* S_LThreadCurrentChunk;
	static thread_local size_t S_LThreadChunkSize;
};



class CASH_LINE_ALIGNED CustomMemoryPool
{
public:

private:
	static constexpr size_t S_MAX_ALLOC_SIZE = 4096;
	static constexpr size_t S_BUCKET_COUNT = 32;
	static constexpr size_t S_BUCKET_SIZE_ARRAY[S_BUCKET_COUNT] = {
		8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128,
		160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
		1280, 1536, 1792, 2048, 2560, 3072, 3584, 4096
	};

	struct CASH_LINE_ALIGNED ThreadCache
	{
		ThreadCache()
		{
			for (auto& bucket : buckets)
				bucket.reserve(S_DEFAULT_BATCH_SIZE);
		}

		static constexpr size_t S_DEFAULT_BATCH_SIZE = 32;

		size_t allocCount = 0;
		size_t deallocCount = 0;
		std::vector<void*> buckets[S_BUCKET_COUNT];
	};

	static CustomMemoryPool& GetInstance()
	{
		static CustomMemoryPool S_Instance;
		return S_Instance;
	}

	static size_t GetBucketIndex(size_t size)
	{
		for (size_t i = 0; i < S_BUCKET_COUNT; i++)
		{
			if (size <= S_BUCKET_SIZE_ARRAY[i])
				return i;
		}

		return S_BUCKET_COUNT - 1;
	}

	static size_t GetBlockSize(size_t index)
	{
		return S_BUCKET_SIZE_ARRAY[index >= S_BUCKET_COUNT ? S_BUCKET_COUNT - 1 : index];
	}

	void RefillCache(size_t bucketIndex)
	{
		ThreadCache& cache = S_LThreadCache;
		auto& bucket = cache.buckets[bucketIndex];

		for (size_t i = 0; i < ThreadCache::S_DEFAULT_BATCH_SIZE; i++)
		{
			void* memoryBlock = nullptr;
			if (_globalPools[bucketIndex].Pop(memoryBlock) == false)
			{
				size_t blockSize = GetBlockSize(bucketIndex);
				size_t totalSize = blockSize + sizeof(MemoryBlockHeader);

				memoryBlock = PoolAllocator::Allocate(totalSize, CACHE_LINE_SIZE);
				new(memoryBlock)MemoryBlockHeader(blockSize, bucketIndex);

				if (memoryBlock)
					bucket.push_back(memoryBlock);
			}
		}
	}

	void FlushCache(size_t bucketIndex)
	{
		ThreadCache& cache = S_LThreadCache;
		auto& bucket = cache.buckets[bucketIndex];

		size_t halfSize = bucket.size() / 2;
		for (size_t i = halfSize; i < bucket.size(); i++)
			_globalPools[bucketIndex].Push(bucket[i]);

		bucket.resize(halfSize);
	}

public:
	// 메모리 할당
	static void* Allocate(size_t size) {
		return GetInstance().AllocateImpl(size);
	}

	// 메모리 해제
	static void Deallocate(void* ptr) {
		if (ptr == nullptr) return;
		GetInstance().DeallocateImpl(ptr);
	}

	// 캐시 스틸링 기능 설정
	static void EnableCacheStealing(bool enable) {
		GetInstance()._enableCacheStealing = enable;
	}

	// 통계 정보 (디버깅용)
	static void PrintStats() {
		auto& instance = GetInstance();
		printf("Total allocations: %zu, Total deallocations: %zu\n",
			instance._totalAllocCount.load(), instance._totalDeallocCount.load());

		printf("Current thread cache stats - Allocs: %zu, Deallocs: %zu\n",
			S_LThreadCache.allocCount, S_LThreadCache.deallocCount);
	}

private:
	void* AllocateImpl(size_t allocSize)
	{
		if (allocSize > S_MAX_ALLOC_SIZE)
		{
			size_t totalSize = allocSize + sizeof(MemoryBlockHeader);
			void* memoryBlock = PoolAllocator::Allocate(totalSize, CACHE_LINE_SIZE);
			auto header = new(memoryBlock)MemoryBlockHeader(allocSize, S_BUCKET_COUNT);

			_totalAllocCount.fetch_add(1, std::memory_order_relaxed);
			S_LThreadCache.allocCount++;

			return MemoryBlockHeader::AttachHeader(header);
		}

			size_t bucketIndex = GetBucketIndex(allocSize);
			ThreadCache& cache = S_LThreadCache;
			auto& blocks = cache.buckets[bucketIndex];

			// 로컬 캐시가 비었으면 보충
			if (blocks.empty()) {
				RefillCache(bucketIndex);
			}

			// 로컬 캐시에서 블록 가져오기
			if (!blocks.empty()) {
				void* block = blocks.back();
				blocks.pop_back();

				MemoryBlockHeader* header = static_cast<MemoryBlockHeader*>(block);
				header->allocSize = allocSize;
				header->timestamp = ++S_LThreadtimestampCounter;

				_totalAllocCount.fetch_add(1, std::memory_order_relaxed);
				S_LThreadCache.allocCount++;

				return MemoryBlockHeader::AttachHeader(header);
			}

			// 모든 전략이 실패하면 시스템에서 직접 할당
			size_t blockSize = GetBlockSize(bucketIndex);
			size_t totalSize = blockSize + sizeof(MemoryBlockHeader);
			void* block = PoolAllocator::Allocate(totalSize, CACHE_LINE_SIZE);
			MemoryBlockHeader* header = new (block) MemoryBlockHeader(allocSize, bucketIndex);
			header->timestamp = ++S_LThreadtimestampCounter;

			_totalAllocCount.fetch_add(1, std::memory_order_relaxed);
			S_LThreadCache.allocCount++;

			return MemoryBlockHeader::AttachHeader(header);
	};

	void DeallocateImpl(void* ptr) 
	{
		MemoryBlockHeader* header = MemoryBlockHeader::DetachHeader(ptr);
		size_t bucketIndex = header->poolIndex;

		_totalDeallocCount.fetch_add(1, std::memory_order_relaxed);
		S_LThreadCache.deallocCount++;

		// 대형 블록은 직접 시스템 반환
		if (bucketIndex >= S_BUCKET_COUNT) {
			PoolAllocator::Allocate(0); // 강제 할당 방지
			::free(header);
			return;
		}

		// 스레드 로컬 캐시로 반환
		ThreadCache& cache = S_LThreadCache;
		auto& bucket = cache.buckets[bucketIndex];

		// 캐시가 너무 크면 일부를 전역 풀로 반환
		if (bucket.size() >= ThreadCache::S_DEFAULT_BATCH_SIZE * 2) {
			FlushCache(bucketIndex);
		}

		// 타임스탬프 업데이트로 ABA 문제 방지
		header->timestamp = ++S_LThreadtimestampCounter;
		bucket.push_back(header);
	}


private:
	ShardedPool<void*> _globalPools[S_BUCKET_COUNT];
	static thread_local ThreadCache S_LThreadCache;
	static thread_local uint64 S_LThreadtimestampCounter;

	std::atomic<size_t> _totalAllocCount = 0;
	std::atomic<size_t> _totalDeallocCount = 0;

	bool _enableCacheStealing = true;
};



// 전역 할당자 인터페이스
class GlobalMemoryManager {
public:
	static void* Allocate(size_t size) {
		return CustomMemoryPool::Allocate(size);
	}

	static void Deallocate(void* ptr) {
		CustomMemoryPool::Deallocate(ptr);
	}

	static void PrintStats() {
		CustomMemoryPool::PrintStats();
	}

	static void EnableCacheStealing(bool enable) {
		CustomMemoryPool::EnableCacheStealing(enable);
	}
};
