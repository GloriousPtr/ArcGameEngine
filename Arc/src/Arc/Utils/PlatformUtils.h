#pragma once

namespace ArcEngine
{
	class FileDialogs
	{
	public:
		// These return empty strings if cancelled
		static eastl::string OpenFolder();
		static eastl::string OpenFile(const char* filter);
		static eastl::string SaveFile(const char* filter);
		static void OpenFolderAndSelectItem(const char* path);
		static void OpenFileWithProgram(const char* path);
		static void OpenSolutionWithVS(const char* path);
	};
}
