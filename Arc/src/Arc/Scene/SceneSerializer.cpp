#include "arcpch.h"
#include "Arc/Scene/SceneSerializer.h"

#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Components.h"
#include "EntitySerializer.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace ArcEngine
{
	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	void SceneSerializer::Serialize(const eastl::string& filepath) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;

			EntitySerializer::SerializeEntity(out, entity);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath.c_str());
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const eastl::string& filepath) const
	{
		// Not implemented
		ARC_CORE_ASSERT(false, "Cannot serialize {} during runtime", filepath);
	}

	bool SceneSerializer::Deserialize(const eastl::string& filepath) const
	{
		std::ifstream stream(filepath.c_str());
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		eastl::string sceneName = data["Scene"].as<std::string>().c_str();
		ARC_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (const auto& entity : entities)
				EntitySerializer::DeserializeEntity(entity, *m_Scene, true);
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const eastl::string& filepath) const
	{
		// Not implemented
		ARC_CORE_ASSERT(false, "Cannot deserialize {} during runtime", filepath);
		return false;
	}
}
