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
		stream.read(buffer.As<char>(), static_cast<std::streamsize>(size));
		stream.close();
		return buffer;
	}

	void Filesystem::ReadFileText(const std::filesystem::path& filepath, std::string& outString)
	{
		ARC_PROFILE_SCOPE()

		std::ifstream in(filepath.c_str(), std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			const int64_t size = in.tellg();
			if (std::cmp_not_equal(size, -1))
			{
				outString.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(outString.data(), size);
			}
			else
			{
				ARC_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			ARC_CORE_ERROR("Could not open file '{0}'", filepath);
		}
	}

	void Filesystem::WriteFileText(const std::filesystem::path& filepath, const std::string& buffer)
	{
		std::ofstream stream(filepath, std::ios::out);
		stream << buffer.c_str();
		stream.close();
	}
}
