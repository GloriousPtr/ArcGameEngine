#include "arcpch.h"
#include "ExternalConsoleSink.h"

namespace ArcEngine
{
	std::function<void(std::string, LogLevel)> ExternalConsoleSink<std::mutex>::OnFlush = nullptr;
}
