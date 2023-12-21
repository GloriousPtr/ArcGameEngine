#pragma once

#include "Arc/Debug/Profiler.h"

namespace ArcEngine
{
	class StringUtils
	{
	public:
		[[nodiscard]] static eastl::string_view GetExtension(eastl::string_view filepath)
		{
			ARC_PROFILE_SCOPE();

			const auto lastDot = filepath.find_last_of('.');
			return filepath.substr(lastDot + 1, filepath.size() - lastDot);
		}

		[[nodiscard]] static eastl::string_view GetName(eastl::string_view filepath)
		{
			ARC_PROFILE_SCOPE();

			auto lastSlash = filepath.find_last_of("/\\");
			lastSlash = lastSlash == eastl::string::npos ? 0 : lastSlash + 1;
			const auto lastDot = filepath.rfind('.');
			const auto count = lastDot == eastl::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
			return filepath.substr(lastSlash, count);
		}

		[[nodiscard]] static eastl::string_view GetNameWithExtension(eastl::string_view filepath)
		{
			ARC_PROFILE_SCOPE();

			auto lastSlash = filepath.find_last_of("/\\");
			lastSlash = lastSlash == eastl::string::npos ? 0 : lastSlash + 1;
			return filepath.substr(lastSlash, filepath.size());
		}

		static void ReplaceString(eastl::string& subject, eastl::string_view search, eastl::string_view replace)
		{
			size_t pos = 0;
			const size_t replaceLength = replace.length();
			const size_t searchLength = search.length();

			while ((pos = subject.find(search.data(), pos)) != eastl::string::npos)
			{
				subject.replace(pos, searchLength, replace.data());
				pos += replaceLength;
			}
		}
	};
}
