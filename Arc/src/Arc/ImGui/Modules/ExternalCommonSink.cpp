#include "arcpch.h"
#include "ExternalConsoleSink.h"

namespace ArcEngine
{
	eastl::function<void(eastl::string, eastl::string, eastl::string, int32_t, Log::Level)> ExternalConsoleSink::OnFlush;
}
