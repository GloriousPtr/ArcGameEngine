#include "arcpch.h"
#include "ExternalConsoleSink.h"

namespace ArcEngine
{
	eastl::function<void(eastl::string, Log::Level)> ExternalConsoleSink::OnFlush;
}
