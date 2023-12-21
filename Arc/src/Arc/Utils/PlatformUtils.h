#pragma once

namespace ArcEngine
{
	class FileDialogs
	{
	public:
		// These return empty strings if cancelled
		[[nodiscard]] static eastl::string OpenFolder();
		[[nodiscard]] static eastl::string OpenFile(const char* filter);
		[[nodiscard]] static eastl::string SaveFile(const char* filter);
		static void OpenFolderAndSelectItem(const char* path);
		static void OpenFileWithProgram(const char* path);
	};
}
