#include "arcpch.h"
#include "ExternalConsoleSink.h"

namespace ArcEngine
{
	std::function<void(eastl::string_view, const char*, const char*, int32_t, Log::Level)> ExternalConsoleSink::OnFlush;
}
