#include "pch.h"
#include "CustomMemoryPool.h"

thread_local PoolAllocator::MemoryChunk* PoolAllocator::S_LThreadLocalChunks = nullptr;
thread_local PoolAllocator::MemoryChunk* PoolAllocator::S_LThreadCurrentChunk = nullptr;
thread_local size_t PoolAllocator::S_LThreadChunkSize = 0;

// ������ ���� ���� �ʱ�ȭ
thread_local CustomMemoryPool::ThreadCache CustomMemoryPool::S_LThreadCache;
thread_local uint64_t CustomMemoryPool::S_LThreadtimestampCounter = 0;
