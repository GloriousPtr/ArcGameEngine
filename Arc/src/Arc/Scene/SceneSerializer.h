#pragma once

#include "Arc/Scene/Scene.h"

namespace ArcEngine
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const eastl::string& filepath);
		void SerializeRuntime(const eastl::string& filepath);

		bool Deserialize(const eastl::string& filepath);
		bool DeserializeRuntime(const eastl::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};
}
