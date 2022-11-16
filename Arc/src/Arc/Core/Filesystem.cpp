#include "arcpch.h"
#include "Filesystem.h"

#include <fstream>

namespace ArcEngine
{
	Buffer Filesystem::ReadFileBinary(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
			return {};

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();

		if (size == 0)
			return {};

		Buffer buffer(size);
		stream.read(buffer.As<char>(), (std::streamsize)size);
		stream.close();
		return buffer;
	}
}
