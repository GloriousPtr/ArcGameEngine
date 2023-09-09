#include "arcpch.h"

#ifdef ARC_PLATFORM_VISUAL_STUDIO

#include "VisualStudioAccessor.h"

#if defined(__clang__) || defined(__llvm__)
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Weverything"
#elif defined(_MSC_VER)
#	pragma warning(push, 0)
#endif

#include "dte80a.tlh"
#include "dte80.tlh"

#if defined(__clang__) || defined(__llvm__)
#	pragma clang diagnostic pop
#elif defined(_MSC_VER)
#	pragma warning(pop)
#endif

//#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" named_guids raw_interfaces_only auto_rename
//#import "libid:1A31287A-4D7D-413E-8E32-3B374931BD89" named_guids raw_interfaces_only auto_rename

#include "Arc/Project/Project.h"
#include "Arc/Scripting/ProjectBuilder.h"

namespace ArcEngine
{
	using namespace Microsoft::WRL;

	inline static constexpr const char* programId = "VisualStudio.DTE";
	inline static ComPtr<EnvDTE80::DTE2> s_VsInstance = nullptr;
	inline static std::future<void> s_OpenProjectFuture;
	inline static std::future<void> s_OpenFileFuture;

	enum class VisualStudioVersion
	{
		Preview2022, Enterprise2022, Professional2022, Community2022,
	};

	inline static eastl::hash_map<VisualStudioVersion, const char*> s_VisualStudioInstallLocation
	{
		{ VisualStudioVersion::Preview2022,			"C:/Program Files/Microsoft Visual Studio/2022/Preview/Common7/IDE/devenv.exe"		},
		{ VisualStudioVersion::Enterprise2022,		"C:/Program Files/Microsoft Visual Studio/2022/Enterprise/Common7/IDE/devenv.exe"	},
		{ VisualStudioVersion::Professional2022,	"C:/Program Files/Microsoft Visual Studio/2022/Professional/Common7/IDE/devenv.exe" },
		{ VisualStudioVersion::Community2022,		"C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe"	},
	};

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
			std::string name(cBuffer);
			if (name.find(programId) != std::string::npos)
			{
				ComPtr<IUnknown> pUnk;
				result = runningObjectTable->GetObjectW(currentMoniker, &pUnk);
				if (FAILED(result))
					continue;

				ComPtr<EnvDTE80::DTE2> vsInstance = nullptr;
				pUnk->QueryInterface(IID_PPV_ARGS(&vsInstance));

				ComPtr<EnvDTE80::_Solution> solution;
				result = vsInstance->get_Solution(&solution);
				if (FAILED(result))
					continue;

				solution->get_FullName(&buffer);
				wBuffer = buffer;
				cBuffer = wBuffer;
				std::filesystem::path path = cBuffer;
				if (path == solutionPath.c_str())
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

		const auto pInfo = reinterpret_cast<ProcessInfo*>(lParam);
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

	static void RunAndOpenSolutionAndFile(const std::filesystem::path& solutionPath, const char* filepath = nullptr, uint32_t goToLine = 0)
	{
		std::string cmdArgs;
		
		constexpr auto versions = magic_enum::enum_values<VisualStudioVersion>();
		for (const auto version : versions)
		{
			if (std::filesystem::exists(s_VisualStudioInstallLocation[version]))
			{
				cmdArgs = s_VisualStudioInstallLocation[version];
				break;
			}
		}

		if (cmdArgs.empty())
		{
			ARC_CORE_ERROR("Failed to run Visual Studio 2022! Make sure VS2022 Preview, Enterprise, Professional or Community is installed");
			return;
		}
		cmdArgs += " ";

		static PROCESS_INFORMATION processInfo = { nullptr, nullptr, 0, 0 };
		ProcessInfo process = {false, processInfo.dwProcessId};

		bool processShouldRun = process.ID == 0;

		if (!processShouldRun)
			EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&process));

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

		if (goToLine > 0)
		{
			cmdArgs += std::format(" /command \"edit.goto {0}\"", goToLine);
		}

		if (processShouldRun)
		{
			_bstr_t wCmdArgs(cmdArgs.c_str());

			STARTUPINFO startInfo{};
			startInfo.wShowWindow = SW_HIDE;
			uint32_t createFlags = NORMAL_PRIORITY_CLASS | DETACHED_PROCESS;
			CreateProcess(nullptr, wCmdArgs, nullptr, nullptr, FALSE, createFlags, nullptr, nullptr, &startInfo, &processInfo);
			WaitForSingleObject(processInfo.hProcess, INFINITE);

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
					RunAndOpenSolutionAndFile(solutionPath);
				else
					ShowDteWindow(s_VsInstance);
			}
			else
			{
				if (!ShowDteWindow(s_VsInstance))
					RunAndOpenSolutionAndFile(solutionPath);
			}
		});
	}

	void VisualStudioAccessor::OpenFile(const std::string& filepath, uint32_t goToLine, bool selectLine)
	{
		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			return;
		}

		s_OpenFileFuture = std::async(std::launch::async, [filepath, goToLine, selectLine]()
		{
			const auto solutionPath = Project::GetSolutionPath();
			if (!IsDteValid())
			{
				FindAndSetRunningInstance(solutionPath.string());

				if (!IsDteValid())
				{
					RunAndOpenSolutionAndFile(solutionPath, filepath.c_str(), goToLine);
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

			const CComBSTR bstrFilepath(filepath.c_str());
			const CComBSTR bstrKind(EnvDTE80::vsViewKindTextView);
			ComPtr<EnvDTE80::Window> window;
			itemOps->OpenFile(bstrFilepath, bstrKind, &window);

			if (goToLine > 0)
			{
				ComPtr<EnvDTE80::Document> doc;
				result = s_VsInstance->get_ActiveDocument(&doc);
				if (FAILED(result))
					return;

				ComPtr<IDispatch> selectionDispatch;
				result = doc->get_Selection(&selectionDispatch);
				if (FAILED(result))
					return;

				ComPtr<EnvDTE80::TextSelection> selection;
				result = selectionDispatch->QueryInterface(IID_PPV_ARGS(&selection));
				if (FAILED(result))
					return;

				selection->GotoLine(static_cast<long>(goToLine), selectLine);
			}

			if (window)
				window->put_Visible(VARIANT_TRUE);
		});
	}

	void VisualStudioAccessor::AddFile(const std::string& filepath, bool open)
	{
		ProjectBuilder::GenerateProjectFiles(nullptr);
		if (open)
			OpenFile(filepath);
	}

	void VisualStudioAccessor::AttachDebugger()
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
					RunAndOpenSolutionAndFile(solutionPath);
				else
					ShowDteWindow(s_VsInstance);
			}
			else
			{
				if (!ShowDteWindow(s_VsInstance))
					RunAndOpenSolutionAndFile(solutionPath);
			}

			if (IsDteValid())
			{
				CComBSTR cmd = "ArcVSExtension.AttachToDebuggerCommand";
				CComBSTR args = "";
				HRESULT hr = s_VsInstance->ExecuteCommand(cmd, args);
				if (FAILED(hr))
					ARC_CORE_ERROR("Failed to attach visual studio debugger! Make sure ArcVSExtension is installed.");
			}
		});
	}

	void VisualStudioAccessor::DetachDebugger()
	{
		if (IsDteValid())
		{
			CComBSTR cmd = "Debug.StopDebugging";
			CComBSTR args = "";
			s_VsInstance->ExecuteCommand(cmd, args);
		}
	}
}

#endif //ARC_PLATFORM_VISUAL_STUDIO
