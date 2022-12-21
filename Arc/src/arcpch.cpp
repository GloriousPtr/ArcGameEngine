#include "arcpch.h"

size_t g_ArcAllocationSize = 0;

namespace ArcEngine::Allocation
{
	size_t GetSize() { return g_ArcAllocationSize; }
}

void operator delete(void* ptr, size_t size) noexcept
{
	g_ArcAllocationSize -= size;
	free(ptr);
}

void* operator new(size_t size)
{
	g_ArcAllocationSize += size;
	return malloc(size);
}
