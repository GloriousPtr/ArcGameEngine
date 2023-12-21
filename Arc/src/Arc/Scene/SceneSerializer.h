#pragma once

namespace ArcEngine
{
	class Scene;

	class SceneSerializer
	{
	public:
		explicit SceneSerializer(const Ref<Scene>& scene);

		void Serialize(eastl::string_view filepath) const;
		void SerializeRuntime(eastl::string_view filepath) const;

		[[nodiscard]] bool Deserialize(eastl::string_view filepath) const;
		[[nodiscard]] bool DeserializeRuntime(eastl::string_view filepath) const;
	private:
		Ref<Scene> m_Scene;
	};
}
