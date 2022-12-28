#include "arcpch.h"

#ifdef ARC_PLATFORM_WINDOWS

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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>
#include <atlbase.h>

//#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" named_guids raw_interfaces_only auto_rename
//#import "libid:1A31287A-4D7D-413E-8E32-3B374931BD89" named_guids raw_interfaces_only auto_rename

#include <future>

#include "Arc/Project/Project.h"

namespace ArcEngine
{
	using namespace Microsoft::WRL;

	inline static constexpr const char* programId = "VisualStudio.DTE";
	inline static constexpr const char* projGenId = "vs2022";
	inline static ComPtr<EnvDTE80::DTE2> s_VsInstance = nullptr;
	inline static std::future<void> s_OpenProjectFuture;
	inline static std::future<void> s_OpenFileFuture;

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
		constexpr const char* vs2022Enterprise = "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/Common7/IDE/devenv.exe";
		constexpr const char* vs2022Professional = "C:/Program Files/Microsoft Visual Studio/2022/Professional/Common7/IDE/devenv.exe";
		constexpr const char* vs2022Community = "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe";

		std::string cmdArgs;
		if (std::filesystem::exists(vs2022Enterprise))
		{
			cmdArgs = vs2022Enterprise;
		}
		else if (std::filesystem::exists(vs2022Professional))
		{
			cmdArgs = vs2022Professional;
		}
		else if (std::filesystem::exists(vs2022Community))
		{
			cmdArgs = vs2022Community;
		}
		else
		{
			ARC_CORE_ERROR("Failed to run Visual Studio 2022! Make sure VS2022 Enterprise, Professional or Community is installed");
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
			CloseHandle(processInfo.hThread);
			CloseHandle(processInfo.hProcess);
		}
	}

	static bool BuildSolutionUsingProcess(const std::filesystem::path& solutionPath, const std::string& buildConfig)
	{
		constexpr const char* buildFlags = ""
			" -nologo"																	// no microsoft branding in console
			" -noconlog"																// no console logs
			//" -t:rebuild"																// rebuild the project
			" -m"																		// multi-process build
			" -flp1:Verbosity=minimal;logfile=AssemblyBuildErrors.log;errorsonly"		// dump errors in AssemblyBuildErrors.log file
			" -flp2:Verbosity=minimal;logfile=AssemblyBuildWarnings.log;warningsonly";	// dump warnings in AssemblyBuildWarnings.log file

		std::string buildCommand = "dotnet.exe msbuild ";

		buildCommand += "\"";
		buildCommand += solutionPath.string();
		buildCommand += "\"";

		buildCommand += " /property:Configuration=";
		buildCommand += buildConfig;

		buildCommand += buildFlags;

		const _bstr_t wCmdArgs(buildCommand.c_str());

		STARTUPINFO startInfo = {};
		PROCESS_INFORMATION processInfo = { nullptr, nullptr, 0, 0 };
		constexpr uint32_t createFlags = HIGH_PRIORITY_CLASS;
		const HRESULT result = CreateProcess(nullptr, wCmdArgs, nullptr, nullptr, FALSE, createFlags, nullptr, nullptr, &startInfo, &processInfo);
		WaitForSingleObject(processInfo.hProcess, INFINITE);

		bool failed = FAILED(result);
		if (failed)
			ARC_CORE_ERROR("Failed to build solution: {}", GetLastError());

		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);

		// Errors
		{
			FILE* errors = nullptr;
			fopen_s(&errors, "AssemblyBuildErrors.log", "r");

			char buffer[4096];
			if (errors != nullptr)
			{
				while (fgets(buffer, sizeof(buffer), errors))
				{
					size_t newLine = std::string_view(buffer).size() - 1;
					buffer[newLine] = '\0';
					ARC_APP_ERROR(buffer);
					failed = true;
				}

				fclose(errors);
			}
		}

		// Warnings
		{
			FILE* warns;
			fopen_s(&warns, "AssemblyBuildWarnings.log", "r");

			char buffer[1024];
			if (warns != nullptr)
			{
				while (fgets(buffer, sizeof(buffer), warns))
				{
					size_t newLine = std::string_view(buffer).size() - 1;
					buffer[newLine] = '\0';
					ARC_APP_WARN(buffer);
				}

				fclose(warns);
			}
		}
		
		return !failed;
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

			CComBSTR bstrFilepath(filepath.c_str());
			CComBSTR bstrKind(EnvDTE80::vsViewKindTextView);
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

	bool VisualStudioAccessor::AddFile(const std::string& filepath, bool open)
	{
		bool success = GenerateProjectFiles();
		if (open)
			OpenFile(filepath);
		return success;
	}

	bool VisualStudioAccessor::GenerateProjectFiles()
	{
		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			return false;
		}

		std::filesystem::path premakeExePath = "../vendor/premake/bin/premake5.exe";
		if (!std::filesystem::exists(premakeExePath))
		{
			ARC_CORE_ERROR("Failed to locate premake5.exe to generate solution files");
			return false;
		}

		std::filesystem::path projectFilepath = Project::GetProjectDirectory() / "premake5.lua";
		if (!std::filesystem::exists(projectFilepath))
		{
			ARC_CORE_ERROR("Failed to locate premake5.lua at {}", projectFilepath);
			return false;
		}

		std::string premakeCommand = "\"";
		premakeCommand += premakeExePath.string();
		premakeCommand += "\"";

		premakeCommand += " --file=";

		premakeCommand += "\"";
		premakeCommand += projectFilepath.string();
		premakeCommand += "\"";

		premakeCommand += " --scripts=";
		premakeCommand += "\"";
		premakeCommand += std::filesystem::current_path().string();
		premakeCommand += "\"";

		premakeCommand += " ";
		premakeCommand += projGenId;

		_bstr_t wCmdArgs(premakeCommand.c_str());

		STARTUPINFO startInfo = {};
		PROCESS_INFORMATION processInfo = { nullptr, nullptr, 0, 0 };
		constexpr uint32_t createFlags = HIGH_PRIORITY_CLASS;
		HRESULT result = CreateProcess(nullptr, wCmdArgs, nullptr, nullptr, FALSE, createFlags, nullptr, nullptr, &startInfo, &processInfo);
		WaitForSingleObject(processInfo.hProcess, INFINITE);

		bool failed = FAILED(result);
		if (failed)
			ARC_CORE_ERROR("Failed to generate solution files: {}", GetLastError());

		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);

		return !failed;
	}
	
	bool VisualStudioAccessor::BuildSolution(const std::function<void()>& onComplete)
	{
		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			return false;
		}

		std::filesystem::path solutionPath = Project::GetSolutionPath();
		if (!IsDteValid())
			FindAndSetRunningInstance(solutionPath.string());

		bool success = BuildSolutionUsingProcess(solutionPath, Project::GetBuildConfigString().data());

		if (onComplete)
			onComplete();

		return success;
	}
}

#endif
