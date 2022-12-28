#include "arcpch.h"

#ifdef ARC_PLATFORM_LINUX

#include "Arc/Utils/PlatformUtils.h"

namespace ArcEngine
{
	std::string FileDialogs::OpenFolder()
	{
		return "";
	}

	std::string FileDialogs::OpenFile(const char* filter)
	{
		return "";
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		return "";
	}

	void FileDialogs::OpenFolderAndSelectItem(const char* path)
	{
	}

	void FileDialogs::OpenFileWithProgram(const char* path)
	{
	}
}

#endif
