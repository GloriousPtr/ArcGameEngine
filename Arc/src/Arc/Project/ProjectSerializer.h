#pragma once

#include "Project.h"

namespace ArcEngine
{
	class ProjectSerializer
	{
	public:
		explicit ProjectSerializer(Ref<Project> project);

		bool Serialize(const std::filesystem::path& filepath) const;
		bool Deserialize(const std::filesystem::path& filepath) const;

	private:
		Ref<Project> m_Project;
	};
}
