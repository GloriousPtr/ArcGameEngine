#include "arcpch.h"
#include "ExternalConsoleSink.h"

namespace ArcEngine
{
	std::function<void(std::string, std::string, std::string, int32_t, Log::Level)> ExternalConsoleSink::OnFlush;
}
