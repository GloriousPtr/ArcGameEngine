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

void* operator new[](
	[[maybe_unused]] size_t size,
	[[maybe_unused]] const char* pName,
	[[maybe_unused]] int flags,
	[[maybe_unused]] unsigned debugFlags,
	[[maybe_unused]] const char* file,
	[[maybe_unused]] int line)
{
	g_ArcAllocationSize += size;
	return malloc(size);
}

void* operator new[](
	[[maybe_unused]] size_t size,
	[[maybe_unused]] size_t alignment,
	[[maybe_unused]] size_t alignmentOffset,
	[[maybe_unused]] const char* pName,
	[[maybe_unused]] int flags,
	[[maybe_unused]] unsigned debugFlags,
	[[maybe_unused]] const char* file,
	[[maybe_unused]] int line)
{
	g_ArcAllocationSize += size;
	return malloc(size);
}

void* operator new(size_t size)
{
	g_ArcAllocationSize += size;
	return malloc(size);
}
