#include "arcpch.h"
#include "VisualStudioAccessor.h"

#include "dte80a.tlh"
#include "dte80.tlh"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wrl.h>
#include <atlbase.h>

//#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" named_guids raw_interfaces_only auto_rename
//#import "libid:1A31287A-4D7D-413E-8E32-3B374931BD89" named_guids raw_interfaces_only auto_rename

#include <future>

#include "Arc/Project/Project.h"

namespace ArcEngine
{
	using namespace Microsoft::WRL;

	static constexpr const char* programId = "VisualStudio.DTE";
	static ComPtr<EnvDTE80::DTE2> s_VsInstance = nullptr;
	static std::future<void> s_OpenProjectFuture;
	static std::future<void> s_OpenFileFuture;

	static bool FindAndSetRunningInstance(const std::string& solutionPath)
	{
		IRunningObjectTable* runningObjectTable = nullptr;
		HRESULT result = GetRunningObjectTable(0, &runningObjectTable);
		if (FAILED(result))
			return false;

		IEnumMoniker* monikerTable = nullptr;
		runningObjectTable->EnumRunning(&monikerTable);
		monikerTable->Reset();

		IBindCtx* bindCtx = nullptr;
		result = CreateBindCtx(0, &bindCtx);
		if (FAILED(result))
			return false;

		IMoniker* currentMoniker;
		while (monikerTable->Next(1, &currentMoniker, nullptr) == S_OK)
		{
			BSTR buffer;
			currentMoniker->GetDisplayName(bindCtx, nullptr, &buffer);
			_bstr_t wBuffer = buffer;
			const char* cBuffer = wBuffer;
			eastl::string name(cBuffer);
			if (name.find(programId) != eastl::string::npos)
			{
				ComPtr<IUnknown> pUnk;
				result = runningObjectTable->GetObjectW(currentMoniker, &pUnk);
				if (FAILED(result))
					continue;

				ComPtr<EnvDTE80::DTE2> vsInstance = nullptr;
				pUnk->QueryInterface(__uuidof(EnvDTE80::DTE2), (void**)&vsInstance);

				ComPtr<EnvDTE80::_Solution> solution;
				result = vsInstance->get_Solution(&solution);
				if (FAILED(result))
					continue;

				solution->get_FullName(&buffer);
				wBuffer = buffer;
				cBuffer = wBuffer;
				std::filesystem::path path = cBuffer;
				if (path == solutionPath)
				{
					s_VsInstance = vsInstance;
					return true;
				}
			}
		}

		return false;
	}

	static bool IsDteValid()
	{
		if (s_VsInstance)
		{
			ComPtr<EnvDTE80::Window> mainWindow = nullptr;
			if (SUCCEEDED(s_VsInstance->get_MainWindow(&mainWindow)))
				return true;
		}

		return false;
	}

	static bool ShowDteWindow(const ComPtr<EnvDTE80::DTE2>& dte)
	{
		ComPtr<EnvDTE80::Window> mainWindow = nullptr;
		if (SUCCEEDED(dte->get_MainWindow(&mainWindow)))
		{
			mainWindow->Activate();
			mainWindow->put_Visible(VARIANT_TRUE);
			return true;
		}

		return false;
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

	void RunAndOpenSolutionAndFile(const std::filesystem::path& solutionPath, const char* filepath)
	{
		static PROCESS_INFORMATION processInfo = { nullptr, nullptr, 0, 0 };
		ProcessInfo process = {false, processInfo.dwProcessId};

		std::string cmdArgs = "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe ";

		bool processShouldRun = process.ID == 0;

		if (!processShouldRun)
			EnumWindows(EnumWindowsProc, (LPARAM)&process);

		if (!process.Found)
		{
			if (std::filesystem::exists(solutionPath))
			{
				cmdArgs += "\"";
				cmdArgs += solutionPath.string();
				cmdArgs += "\"";
				processShouldRun = true;
			}
		}

		if (filepath)
		{
			cmdArgs += " \"";
			cmdArgs += filepath;
			cmdArgs += "\"";
			processShouldRun = true;
		}

		if (processShouldRun)
		{
			_bstr_t wCmdArgs(cmdArgs.c_str());

			STARTUPINFO startInfo =
			{
				sizeof(STARTUPINFO),
				nullptr, nullptr, nullptr,
				(::DWORD)CW_USEDEFAULT,
				(::DWORD)CW_USEDEFAULT,
				(::DWORD)CW_USEDEFAULT,
				(::DWORD)CW_USEDEFAULT,
				(::DWORD)0, (::DWORD)0, (::DWORD)0,
				(::DWORD)0,
				SW_HIDE,
				0, nullptr,
				nullptr, nullptr, nullptr
			};
			uint32_t CreateFlags = NORMAL_PRIORITY_CLASS | DETACHED_PROCESS;
			CreateProcess(nullptr, wCmdArgs, nullptr, nullptr, FALSE, CreateFlags, nullptr, nullptr, &startInfo, &processInfo);
			CloseHandle(processInfo.hThread);
			CloseHandle(processInfo.hProcess);
		}
	}

	void VisualStudioAccessor::RunVisualStudio()
	{
		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			return;
		}

		s_OpenProjectFuture = std::async(std::launch::async, []()
		{
			const auto solutionPath = Project::GetSolutionPath();
			if (!IsDteValid())
			{
				FindAndSetRunningInstance(solutionPath.string());

				if (!IsDteValid())
					RunAndOpenSolutionAndFile(solutionPath, nullptr);
				else
					ShowDteWindow(s_VsInstance);
			}
			else
			{
				if (!ShowDteWindow(s_VsInstance))
					RunAndOpenSolutionAndFile(solutionPath, nullptr);
			}
		});
	}

	void VisualStudioAccessor::OpenFile(const eastl::string& filepath)
	{
		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			return;
		}

		s_OpenFileFuture = std::async(std::launch::async, [filepath]()
		{
			const auto solutionPath = Project::GetSolutionPath();
			if (!IsDteValid())
			{
				FindAndSetRunningInstance(solutionPath.string());

				if (!IsDteValid())
				{
					RunAndOpenSolutionAndFile(solutionPath, filepath.c_str());
					return;
				}
			}

			if (!IsDteValid())
				FindAndSetRunningInstance(solutionPath.string());

			ShowDteWindow(s_VsInstance);
			ComPtr<EnvDTE80::ItemOperations> itemOps;
			HRESULT result = s_VsInstance->get_ItemOperations(&itemOps);
			if (FAILED(result))
				return;

			CComBSTR bstrFilepath(filepath.c_str());
			CComBSTR bstrKind(EnvDTE80::vsViewKindTextView);
			ComPtr<EnvDTE80::Window> window;
			itemOps->OpenFile(bstrFilepath, bstrKind, &window);
			if (window)
				window->put_Visible(VARIANT_TRUE);
		});
	}
}
