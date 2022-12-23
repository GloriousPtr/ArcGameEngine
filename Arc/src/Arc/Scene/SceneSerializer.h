#pragma once

namespace ArcEngine
{
	class Scene;

	class SceneSerializer
	{
	public:
		explicit SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath) const;
		void SerializeRuntime(const std::string& filepath) const;

		[[nodiscard]] bool Deserialize(const std::string& filepath) const;
		[[nodiscard]] bool DeserializeRuntime(const std::string& filepath) const;
	private:
		Ref<Scene> m_Scene;
	};
}
