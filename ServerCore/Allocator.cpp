#include "pch.h"
#include "Allocator.h"
#include "MemoryPool.h"

void* Allocator::Allocate(uint32 size)
{
	return GMemoryPool->Allocate(size);
}

void Allocator::Deallocate(void* ptr)
{
	GMemoryPool->Release(ptr);
}