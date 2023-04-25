#include "arcpch.h"

#ifdef ARC_PLATFORM_WINDOWS

#include <comutil.h>

#include "Arc/Project/Project.h"
#include "Arc/Scripting/ProjectBuilder.h"

namespace ArcEngine
{
	inline static std::future<void> s_BuildProjectFuture;

	bool ProjectBuilder::GenerateProjectFiles()
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

		premakeCommand += " --dotnet=msnet";

		premakeCommand += " --file=";

		premakeCommand += "\"";
		premakeCommand += projectFilepath.string();
		premakeCommand += "\"";

		premakeCommand += " --scripts=";
		premakeCommand += "\"";
		premakeCommand += std::filesystem::current_path().string();
		premakeCommand += "\"";

		premakeCommand += " vs2022";

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

	void ProjectBuilder::BuildProject(bool async, const std::function<void(bool)>& onComplete)
	{
		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			if (onComplete)
				onComplete(false);
			return;
		}

		const std::filesystem::path solutionPath = Project::GetSolutionPath();
		const std::string buildConfig = static_cast<std::string>(Project::GetBuildConfigString());

		s_BuildProjectFuture = std::async(async ? std::launch::async : std::launch::deferred, [solutionPath, buildConfig, onComplete]()
		{
			constexpr const char* buildFlags = ""
				" -nologo"																	// no microsoft branding in console
				" -noconlog"																// no console logs
				//" -t:rebuild"																// rebuild the project
				" -m"																		// multi-process build
				" -flp1:Verbosity=minimal;logfile=AssemblyBuildErrors.log;errorsonly"		// dump errors in AssemblyBuildErrors.log file
				" -flp2:Verbosity=minimal;logfile=AssemblyBuildWarnings.log;warningsonly";	// dump warnings in AssemblyBuildWarnings.log file

			std::string buildCommand = "dotnet.exe build ";

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

			bool success = SUCCEEDED(result);
			if (!success)
				ARC_CORE_ERROR("Failed to build solution: {}", GetLastError());

			CloseHandle(processInfo.hThread);
			CloseHandle(processInfo.hProcess);

			// Errors
			{
				FILE* errors = nullptr;
				fopen_s(&errors, "AssemblyBuildErrors.log", "r");

				if (errors != nullptr)
				{
					char buffer[4096];
					while (fgets(buffer, sizeof(buffer), errors))
					{
						const size_t newLine = std::string_view(buffer).size() - 1;
						buffer[newLine] = '\0';
						ARC_CORE_ERROR(buffer);
						success = false;
					}

					fclose(errors);
				}
			}

			// Warnings
			{
				FILE* warns;
				fopen_s(&warns, "AssemblyBuildWarnings.log", "r");

				if (warns != nullptr)
				{
					char buffer[1024];
					while (fgets(buffer, sizeof(buffer), warns))
					{
						const size_t newLine = std::string_view(buffer).size() - 1;
						buffer[newLine] = '\0';
						ARC_APP_WARN(buffer);
					}

					fclose(warns);
				}
			}

			if (onComplete)
				onComplete(success);
		});

		if (!async)
			s_BuildProjectFuture.get();
	}
}

#endif
