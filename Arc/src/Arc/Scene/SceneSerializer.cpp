#include "arcpch.h"
#include "Arc/Scene/SceneSerializer.h"

#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Scene.h"
#include "Arc/Utils/Stopwatch.h"
#include "EntitySerializer.h"

namespace ArcEngine
{
	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	void SceneSerializer::Serialize(eastl::string_view filepath) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		const auto view = m_Scene->m_Registry.view<IDComponent>();
		for (auto it = view.rbegin(); it != view.rend(); ++it)
		{
			const Entity entity = { *it, m_Scene.get() };
			if (!entity)
				return;

			EntitySerializer::SerializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath.begin());
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(eastl::string_view) const
	{
		// Not implemented
	}

	bool SceneSerializer::Deserialize(eastl::string_view filepath) const
	{
		Stopwatch stopwatch;

		std::ifstream stream(filepath.begin());
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		auto sceneName = data["Scene"].as<std::string>();
		ARC_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		if (auto entities = data["Entities"])
		{
			for (const auto& entity : entities)
				EntitySerializer::DeserializeEntity(entity, *m_Scene, true);
		}

		ARC_CORE_TRACE("Deserializing scene '{}' completed: {}s", sceneName, stopwatch.Stop());

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(eastl::string_view) const
	{
		// Not implemented
		return false;
	}
}
