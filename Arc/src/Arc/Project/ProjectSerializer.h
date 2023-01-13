#pragma once

namespace ArcEngine
{
	class Project;

	class ProjectSerializer
	{
	public:
		explicit ProjectSerializer(const Ref<Project>& project);

		[[nodiscard]] bool Serialize(const std::filesystem::path& filepath) const;
		[[nodiscard]] bool Deserialize(const std::filesystem::path& filepath) const;

	private:
		Ref<Project> m_Project;
	};
}
