#pragma once

namespace ArcEngine
{
	class FileDialogs
	{
	public:
		// These return empty strings if cancelled
		static std::string OpenFolder();
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
		static void OpenFolderAndSelectItem(const char* path);
		static void OpenFileWithProgram(const char* path);
	};
}
