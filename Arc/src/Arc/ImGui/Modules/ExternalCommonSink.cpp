#include "arcpch.h"
#include "ExternalConsoleSink.h"

namespace ArcEngine
{
	std::function<void(std::string, Log::Level)> ExternalConsoleSink<std::mutex>::OnFlush = nullptr;
}
