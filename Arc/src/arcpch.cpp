#include "arcpch.h"

size_t g_ArcAllocationSize = 0;

namespace ArcEngine::Allocation
{
	size_t GetSize() { return g_ArcAllocationSize; }
}

void* operator new(size_t size)
{
	if (size == 0)
		++size;

	g_ArcAllocationSize += size;
	return std::malloc(size);
}

void operator delete(void* ptr, size_t size) noexcept
{
	g_ArcAllocationSize -= size;
	std::free(ptr);
}
