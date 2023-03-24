#include "arcpch.h"
#include "Filesystem.h"

#include <fstream>

namespace ArcEngine
{
	bool Filesystem::IsPartOfDirectoryTree(const std::filesystem::path& filePath, const std::filesystem::path& rootPath)
	{
		const auto relPath = std::filesystem::relative(filePath, rootPath);
		return relPath.empty() || relPath.string().front() != '.';
	}

	Buffer Filesystem::ReadFileBinary(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
			return {};

		const std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		const uint64_t size = end - stream.tellg();

		if (size == 0)
			return {};

		Buffer buffer(size);
		stream.read(buffer.As<char>(), static_cast<std::streamsize>(size));
		stream.close();
		return buffer;
	}

	bool Filesystem::WriteFileBinary(const std::filesystem::path& filepath, Buffer& buffer)
	{
		const auto parentPath = filepath.parent_path();
		if (!std::filesystem::exists(parentPath))
			std::filesystem::create_directories(parentPath);

		std::ofstream stream(filepath, std::ios::binary | std::ios::out);

		if (!stream)
			return false;

		stream.write(buffer.As<const char>(), buffer.Size);
		stream.close();
		return true;
	}

	std::string Filesystem::ReadFileText(const std::filesystem::path& filepath)
	{
		ARC_PROFILE_SCOPE()

		std::ifstream in(filepath.c_str(), std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			const int64_t size = in.tellg();
			if (std::cmp_not_equal(size, -1))
			{
				std::string ret;
				ret.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(ret.data(), size);
				return ret;
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

		return "";
	}

	void Filesystem::WriteFileText(const std::filesystem::path& filepath, const std::string& buffer)
	{
		std::ofstream stream(filepath, std::ios::out);
		stream << buffer.c_str();
		stream.close();
	}
}
