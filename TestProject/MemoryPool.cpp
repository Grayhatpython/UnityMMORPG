#include "pch.h"
#include "MemoryPool.h"

void ThreadLocalCache::Refill(size_t bucketIndex, size_t size, size_t count)
{
	auto& bucket = _buckets[bucketIndex];

	const auto refillCount = std::min(S_MAX_CACHE_BLOCK_SIZE - bucket.count, count);

	for (auto i = 0; i < refillCount; i++)
	{
		//	TODO
		void* memory = GMemoryPool.AllocateFromGlobalMemoryPool(size);
		if (memory)
			bucket.blocks[bucket.count++] = memory;
		else
			break;
	}

}
