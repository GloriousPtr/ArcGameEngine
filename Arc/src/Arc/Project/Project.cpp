#include "arcpch.h"
#include "Project.h"

#include "Arc/Core/Filesystem.h"
#include "Arc/Utils/StringUtils.h"
#include "ProjectSerializer.h"

namespace ArcEngine
{
	bool Project::IsPartOfProject(const std::filesystem::path& filepath)
	{
		ARC_CORE_ASSERT(s_ActiveProject);
		return !filepath.empty() && Filesystem::IsPartOfDirectoryTree(filepath, Project::GetProjectDirectory());
	}

	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateRef<Project>();
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		const Ref<Project> project = CreateRef<Project>();

		const ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			project->m_ProjectDirectory = path.parent_path();
			s_ActiveProject = project;

			if (!std::filesystem::exists(GetAssetDirectory()))
				std::filesystem::create_directory(GetAssetDirectory());

			if (!std::filesystem::exists(GetScriptModuleDirectory()))
				std::filesystem::create_directory(GetScriptModuleDirectory());

			const std::filesystem::path premakeFilepath = GetProjectDirectory() / "premake5.lua";
			if (!std::filesystem::exists(premakeFilepath))
			{
				std::string buffer = Filesystem::ReadFileText("Resources/Templates/PremakeProjectTemplate.txt");
				StringUtils::ReplaceString(buffer, "{PROJECT_NAME}", s_ActiveProject->GetConfig().Name);
				Filesystem::WriteFileText(premakeFilepath, buffer);
			}

			return s_ActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		const ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path))
		{
			s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}

		return false;
	}
}
