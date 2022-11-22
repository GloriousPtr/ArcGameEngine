#pragma once

#include <filesystem>
#include <EASTL/string.h>

#include "Buffer.h"

namespace ArcEngine
{
	class Filesystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
		static void ReadFileText(const std::filesystem::path& filepath, eastl::string& outString);
		static void WriteFileText(const std::filesystem::path& filepath, const eastl::string& buffer);
	};
}
