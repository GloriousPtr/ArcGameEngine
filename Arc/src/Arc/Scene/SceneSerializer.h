#pragma once

#include "Arc/Scene/Scene.h"

namespace ArcEngine
{
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
