#include "arcpch.h"

std::atomic<uint64_t> g_ArcAllocationSize = 0;

namespace ArcEngine::Allocation
{
	uint64_t GetSize() { return g_ArcAllocationSize; }

	void* New(size_t size)
	{
		if (size == 0)
			++size;

		g_ArcAllocationSize += size;
		return malloc(size);
	}

	void Free(void* ptr, size_t size)
	{
		g_ArcAllocationSize -= size;
		free(ptr);
	}
}

void* operator new(size_t size)
{
	if (size == 0)
		++size;

	g_ArcAllocationSize += size;
	return malloc(size);
}

void operator delete(void* ptr, size_t size) noexcept
{
	g_ArcAllocationSize -= size;
	free(ptr);
}

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	if (size == 0)
		++size;

	g_ArcAllocationSize += size;
	return malloc(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	if (size == 0)
		++size;

	g_ArcAllocationSize += size;
	return malloc(size);
}
