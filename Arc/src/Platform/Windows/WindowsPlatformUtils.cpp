#include "arcpch.h"
#include "Arc/Utils/PlatformUtils.h"

#include <ShlObj_core.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <comdef.h>

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
		const auto* pidl = SHBrowseForFolder(&bi);
		if (pidl)
		{
			if (SHGetPathFromIDList(pidl, szTitle))
			{
				_bstr_t b(szTitle);
				const char* c = b;
				return c;
			}
		}
		return "";
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
		return "";
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
		return "";
	}

	void FileDialogs::OpenFolderAndSelectItem(const char* path)
	{
		_bstr_t widePath(path);
		if (ITEMIDLIST* pidl = ILCreateFromPath(widePath))
		{
			SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
			ILFree(pidl);
		}
	}

	void FileDialogs::OpenFileWithProgram(const char* path)
	{
		_bstr_t widePath(path);
		ShellExecute(nullptr, L"open", widePath, nullptr, nullptr, SW_RESTORE);
	}

	struct ProcessInfo
	{
		bool Found = false;
		DWORD ID = 0;
	};

	BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
	{
		DWORD dwPID;
		GetWindowThreadProcessId(hwnd, &dwPID);

		auto pInfo = (ProcessInfo*)lParam;
		if (dwPID == pInfo->ID && GetWindow(hwnd, GW_OWNER) == nullptr && IsWindowVisible(hwnd))
		{
			WINDOWPLACEMENT place = {};
			place.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hwnd, &place);

			switch (place.showCmd)
			{
				case SW_SHOWMAXIMIZED:
					ShowWindow(hwnd, SW_SHOWMAXIMIZED);
					break;
				case SW_SHOWMINIMIZED:
					ShowWindow(hwnd, SW_RESTORE);
					break;
				default:
					ShowWindow(hwnd, SW_NORMAL);
					break;
			}

			SetForegroundWindow(hwnd);
			pInfo->Found = true;

			return FALSE;
		}

		return TRUE;
	}

	void FileDialogs::OpenSolutionWithVS(const char* path)
	{
		static PROCESS_INFORMATION processInfo = {nullptr, nullptr, 0, 0};
		ProcessInfo process = { false, processInfo.dwProcessId };
		if (process.ID)
		{
			EnumWindows(EnumWindowsProc, (LPARAM)&process);
			if (process.Found)
				return;
		}

		STARTUPINFO startInfo = {};
		std::string cmdArgs = std::string("devenv.exe ") + path;
		_bstr_t wCmdArgs(cmdArgs.c_str());
		CreateProcess(L"C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe", wCmdArgs, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startInfo, &processInfo);
	}
}
