#include "arcpch.h"

#ifdef ARC_PLATFORM_LINUX

#include "Arc/Scripting/ProjectBuilder.h"

#include "Arc/Project/Project.h"

#include <stdio.h>

namespace ArcEngine
{
	bool ProjectBuilder::GenerateProjectFiles()
	{
		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			return false;
		}

		std::filesystem::path premakeExePath = "../vendor/premake/bin/premake5";
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

		premakeCommand += " --dotnet=mono";

		premakeCommand += " --file=";

		premakeCommand += "\"";
		premakeCommand += projectFilepath.string();
		premakeCommand += "\"";

		premakeCommand += " --scripts=";
		premakeCommand += "\"";
		premakeCommand += std::filesystem::current_path().string();
		premakeCommand += "\"";

		premakeCommand += " ";
		premakeCommand += "gmake2";

		std::system(premakeCommand.c_str());

		return true;
	}

	bool ProjectBuilder::BuildProject(const std::function<void()>& onComplete)
	{
		if (!Project::GetActive())
		{
			ARC_CORE_ERROR("No active project found!");
			return false;
		}

		std::filesystem::path solutionPath = Project::GetProjectDirectory();
		std::string buildConfig = static_cast<std::string>(Project::GetBuildConfigString());

		// Make expects build config in lowercase
		std::transform(buildConfig.begin(), buildConfig.end(), buildConfig.begin(), [](unsigned char c) { return std::tolower(c); });

		std::string buildCommand = "make -C ";

		buildCommand += "\"";
		buildCommand += solutionPath.string();
		buildCommand += "\"";

		buildCommand += " config=";
		buildCommand += buildConfig;

		buildCommand += " -j$(nproc)";

		std::system(buildCommand.c_str());

		bool failed = false;

		// Errors
		{
			FILE* errors = fopen("AssemblyBuildErrors.log", "r");

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
			FILE* warns = fopen("AssemblyBuildWarnings.log", "r");

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
}

#endif
