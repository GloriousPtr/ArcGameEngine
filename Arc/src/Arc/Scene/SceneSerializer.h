#pragma once

namespace ArcEngine
{
	class Scene;

	class SceneSerializer
	{
	public:
		explicit SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const eastl::string& filepath) const;
		void SerializeRuntime(const eastl::string& filepath) const;

		bool Deserialize(const eastl::string& filepath) const;
		bool DeserializeRuntime(const eastl::string& filepath) const;
	private:
		Ref<Scene> m_Scene;
	};
}
