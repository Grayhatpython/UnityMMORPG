#pragma once

struct Allocator
{
	static void* Allocate(uint32 size);
	static void Deallocate(void* ptr);
};

template<typename T>
class StlAllocator
{
public:
	using value_type = T;

	StlAllocator() { }

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) { }

	T* allocate(size_t count)
	{
		const uint32 size = static_cast<uint32>(count * sizeof(T));
		return static_cast<T*>(Allocator::Allocate(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		Allocator::Deallocate(ptr);
	}
};