#pragma once

#include "Entity.h"

namespace YAML
{
	class Emitter;
	class Node;
}

namespace ArcEngine
{
	class EntitySerializer
	{
	public:
		static void SerializeEntity(YAML::Emitter& out, Entity entity);
		static UUID DeserializeEntity(YAML::Node node, Scene& scene, bool preserveUUID);
		static void SerializeEntityAsPrefab(const char* filepath, Entity entity);
		static bool DeserializeEntityAsPrefab(const char* filepath, Scene& scene);
	};
}
