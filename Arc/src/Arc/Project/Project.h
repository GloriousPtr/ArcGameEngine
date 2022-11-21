#pragma once

#include <filesystem>

#include <EASTL/string.h>

#include "Arc/Core/Base.h"

namespace ArcEngine
{
	struct ProjectConfig
	{
		eastl::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptModulePath;
	};

	class Project
	{
	public:
		static const std::filesystem::path& GetProjectDirectory()
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}

		static std::filesystem::path GetAssetDirectory()
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return GetAssetDirectory() / path;
		}

		static std::filesystem::path GetAssetRelativeFileSystemPath(const std::filesystem::path& path)
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return std::filesystem::relative(path, GetAssetDirectory());
		}

		ProjectConfig& GetConfig() { return m_Config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;

		inline static Ref<Project> s_ActiveProject;
	};
}
