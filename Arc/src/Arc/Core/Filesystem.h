#pragma once

#include <filesystem>

#include "Buffer.h"

namespace ArcEngine
{
	class Filesystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
	};
}
