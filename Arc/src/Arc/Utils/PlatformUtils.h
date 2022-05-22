#pragma once

namespace ArcEngine
{
	class FileDialogs
	{
	public:
		// These return empty strings if cancelled
		static eastl::string OpenFile(const char* filter);
		static eastl::string SaveFile(const char* filter);
	};
}
