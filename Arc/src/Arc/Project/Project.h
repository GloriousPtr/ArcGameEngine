#pragma once

#include "Arc/Core/Base.h"

namespace ArcEngine
{
	struct ProjectConfig
	{
		enum class BuildConfig
		{
			Debug = 0,
			Release,
			Dist
		};
		
		eastl::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory = "Assets";
		std::filesystem::path ScriptModulePath = "Binaries";

		BuildConfig BuildConfiguration = BuildConfig::Debug;
	};

	class Project
	{
		inline static const eastl::hash_map<ProjectConfig::BuildConfig, const char*> s_BuildConfigMap =
		{
			{ ProjectConfig::BuildConfig::Debug,	"Debug"		},
			{ ProjectConfig::BuildConfig::Release,	"Release"	},
			{ ProjectConfig::BuildConfig::Dist,		"Dist"		},
		};

	public:
		[[nodiscard]] static const std::filesystem::path& GetProjectDirectory()
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}

		[[nodiscard]] static std::filesystem::path GetAssetDirectory()
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		[[nodiscard]] static std::filesystem::path GetScriptModuleDirectory()
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.ScriptModulePath;
		}

		[[nodiscard]] static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return GetAssetDirectory() / path;
		}

		[[nodiscard]] static std::filesystem::path GetAssetRelativeFileSystemPath(const std::filesystem::path& path)
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return std::filesystem::relative(path, GetAssetDirectory());
		}

		[[nodiscard]] static std::filesystem::path GetSolutionPath()
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / (s_ActiveProject->GetConfig().Name + ".sln").c_str();
		}

		[[nodiscard]] static eastl::string_view GetBuildConfigString()
		{
			ARC_CORE_ASSERT(s_ActiveProject);
			return s_BuildConfigMap.at(s_ActiveProject->GetConfig().BuildConfiguration);
		}


		[[nodiscard]] ProjectConfig& GetConfig() { return m_Config; }

		[[nodiscard]] static Ref<Project> GetActive() { return s_ActiveProject; }

		[[nodiscard]] static bool IsPartOfProject(const std::filesystem::path& filepath);

		[[nodiscard]] static Ref<Project> New();
		[[nodiscard]] static Ref<Project> Load(const std::filesystem::path& path);
		[[nodiscard]] static bool SaveActive(const std::filesystem::path& path);
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;

		inline static Ref<Project> s_ActiveProject;
	};
}
