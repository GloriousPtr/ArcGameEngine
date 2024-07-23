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

void* operator new[](size_t size, [[maybe_unused]] const char* pName, [[maybe_unused]] int flags, [[maybe_unused]] unsigned debugFlags, [[maybe_unused]] const char* file, [[maybe_unused]] int line)
{
	if (size == 0)
		++size;

	g_ArcAllocationSize += size;
	return malloc(size);
}

void* operator new[](size_t size, [[maybe_unused]] size_t alignment, [[maybe_unused]] size_t alignmentOffset, [[maybe_unused]] const char* pName, [[maybe_unused]] int flags, [[maybe_unused]] unsigned debugFlags, [[maybe_unused]] const char* file, [[maybe_unused]] int line)
{
	if (size == 0)
		++size;

	g_ArcAllocationSize += size;
	return malloc(size);
}
