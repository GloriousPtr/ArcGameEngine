#include "arcpch.h"

#include "Arc/Core/Buffer.h"
#include "Arc/Core/Filesystem.h"
#include "Arc/Utils/StringUtils.h"
#include "Arc/Project/Project.h"
#include "Arc/Scripting/ProjectBuilder.h"

namespace ArcEngine
{
	bool ProjectBuilder::GenerateProjectFiles()
	{
		ARC_PROFILE_SCOPE();

		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			return false;
		}

		std::filesystem::path projectFilepath = Project::GetProjectDirectory() / "premake5.lua";

	#if defined(ARC_PLATFORM_WINDOWS)
		std::filesystem::path premakeExePath = "vendor/premake/bin/premake5.exe";
	#elif defined(ARC_PLATFORM_LINUX)
		std::filesystem::path premakeExePath = "vendor/premake/bin/premake5";
	#else
	#error Platform not supported
	#endif

		if (!std::filesystem::exists(premakeExePath))
		{
			ARC_CORE_ERROR("Failed to locate premake5.exe to generate solution files");
			return false;
		}

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

	#if defined(ARC_PLATFORM_WINDOWS)
		premakeCommand += " vs2022";
		_bstr_t wCmdArgs(premakeCommand.c_str());
		STARTUPINFO startInfo = {};
		PROCESS_INFORMATION processInfo = { nullptr, nullptr, 0, 0 };
		HRESULT result = CreateProcess(nullptr, wCmdArgs, nullptr, nullptr, TRUE, HIGH_PRIORITY_CLASS, nullptr, nullptr, &startInfo, &processInfo);
		WaitForSingleObject(processInfo.hProcess, INFINITE);
		bool success = SUCCEEDED(result);
		if (!success)
			ARC_CORE_ERROR("Failed to generate solution files: {}", GetLastError());
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	#elif ARC_PLATFORM_LINUX
		premakeCommand += " gmake2";
		bool success = std::system(premakeCommand.c_str()) == 0;
	#else
	#error Platform not supported
	#endif

		return success;
	}

	bool ProjectBuilder::BuildProject()
	{
		ARC_PROFILE_SCOPE();

		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			return false;
		}
		
		std::string buildConfig = Project::GetBuildConfigString().data();

	#if defined(ARC_PLATFORM_WINDOWS)
		const std::filesystem::path solutionPath = Project::GetSolutionPath();
	#elif defined(ARC_PLATFORM_LINUX)
		const std::filesystem::path solutionPath = Project::GetProjectDirectory();
	#else
	#error Platform not supported
	#endif

	#if defined(ARC_PLATFORM_WINDOWS)
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
		const HRESULT result = CreateProcess(nullptr, wCmdArgs, nullptr, nullptr, TRUE, HIGH_PRIORITY_CLASS, nullptr, nullptr, &startInfo, &processInfo);
		WaitForSingleObject(processInfo.hProcess, INFINITE);
		bool success = SUCCEEDED(result);
		if (!success)
			ARC_CORE_ERROR("Failed to build solution: {}", GetLastError());

		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	#elif defined(ARC_PLATFORM_LINUX)
		// Make expects build config in lowercase
		std::string config = buildConfig;
		std::transform(config.begin(), config.end(), config.begin(), [](unsigned char c) { return std::tolower(c); });

		std::string buildCommand = "make -C ";

		buildCommand += "\"";
		buildCommand += solutionPath.string();
		buildCommand += "\"";

		buildCommand += " config=";
		buildCommand += config;

		buildCommand += " -j$(nproc)";

		bool success = std::system(buildCommand.c_str()) == 0;
	#else
	#error Platform not supported
	#endif

		// Errors
		{
			ScopedBuffer buf = Filesystem::ReadFileBinary("AssemblyBuildErrors.log");
			char* errors = buf.As<char>();
			char* start = errors + 3;
			uint64_t sz = buf.Size();

			if (errors != nullptr)
			{
				for (uint64_t i = 3; i < sz; ++i)
				{
					if (errors[i] == '\n' || errors[i] == 0)
					{
						errors[i] = 0;
						ARC_CORE_ERROR(start);
						start = &errors[i + 1];
					}
				}
			}
		}

		// Warnings
		{
			ScopedBuffer buf = Filesystem::ReadFileBinary("AssemblyBuildWarnings.log");
			char* warns = buf.As<char>();
			char* start = warns + 3;
			uint64_t sz = buf.Size();

			if (warns != nullptr)
			{
				for (uint64_t i = 3; i < sz; ++i)
				{
					if (warns[i] == '\n' || warns[i] == 0)
					{
						warns[i] = 0;
						ARC_CORE_ERROR(start);
						start = &warns[i + 1];
					}
				}
			}
		}

		return true;
	}
}
