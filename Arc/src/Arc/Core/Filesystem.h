#pragma once

#include "Buffer.h"

namespace ArcEngine
{
	class Filesystem
	{
	public:
		[[nodiscard]] static Buffer ReadFileBinary(const std::filesystem::path& filepath);
		[[nodiscard]] static std::string ReadFileText(const std::filesystem::path& filepath);
		static void WriteFileText(const std::filesystem::path& filepath, const std::string& buffer);
	};
}
