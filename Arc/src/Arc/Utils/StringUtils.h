#pragma once

#include "Arc/Debug/Profiler.h"

namespace ArcEngine
{
	class StringUtils
	{
	public:
		struct StringHash
		{
			using is_transparent = void;
			[[nodiscard]] size_t operator()(const char* txt) const
			{
				return std::hash<std::string_view>{}(txt);
			}

			[[nodiscard]] size_t operator()(std::string_view txt) const
			{
				return std::hash<std::string_view>{}(txt);
			}

			[[nodiscard]] size_t operator()(const std::string& txt) const
			{
				return std::hash<std::string>{}(txt);
			}
		};
#define UM_StringTransparentEquality StringUtils::StringHash, std::equal_to<>

		inline static std::string GetExtension(const std::string& filepath)
		{
			ARC_PROFILE_SCOPE();

			auto lastDot = filepath.find_last_of(".");
			return filepath.substr(lastDot + 1, filepath.size() - lastDot);
		}

		inline static std::string GetName(const std::string& filepath)
		{
			ARC_PROFILE_SCOPE();

			auto lastSlash = filepath.find_last_of("/\\");
			lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
			auto lastDot = filepath.rfind('.');
			auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
			return filepath.substr(lastSlash, count);
		}

		inline static std::string GetNameWithExtension(const std::string& filepath)
		{
			ARC_PROFILE_SCOPE();

			auto lastSlash = filepath.find_last_of("/\\");
			lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
			return filepath.substr(lastSlash, filepath.size());
		}

		inline static void ReplaceString(std::string& subject, const std::string& search, const std::string& replace)
		{
			size_t pos = 0;
			while ((pos = subject.find(search, pos)) != std::string::npos)
			{
				subject.replace(pos, search.length(), replace);
				pos += replace.length();
			}
		}
	};
}
