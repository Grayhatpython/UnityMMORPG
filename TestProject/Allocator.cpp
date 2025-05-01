#include "pch.h"
#include "MemoryPool.h"

void* Allocator::Allocate(uint32_t size)
{
	return GMemoryPool.Allocate(size);
}

void Allocator::Deallocate(void* ptr)
{
	GMemoryPool.Deallocate(ptr);
}