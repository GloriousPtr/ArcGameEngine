#include "arcpch.h"
#include "Arc/Utils/PlatformUtils.h"

#include <ShlObj_core.h>
#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Arc/Core/Application.h"

namespace ArcEngine
{
	eastl::string FileDialogs::OpenFolder()
	{
		WCHAR szTitle[MAX_PATH];
		BROWSEINFO bi;
		bi.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		bi.pidlRoot = nullptr;
		bi.pszDisplayName = szTitle;
		bi.lpszTitle = L"Select a folder containing the response file";
		bi.ulFlags = BIF_RETURNONLYFSDIRS;
		bi.lpfn = nullptr;
		bi.lParam = 0;
		const LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
		if (pidl)
		{
			if (SHGetPathFromIDList(pidl, szTitle))
			{
				std::wstring ws(szTitle);
				std::string str(ws.begin(), ws.end());
				return eastl::string(str.c_str());
			}
		}
		return eastl::string();
	}

	eastl::string FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return eastl::string();
	}

	eastl::string FileDialogs::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return eastl::string();
	}

}
