#pragma once

typedef struct _MonoType MonoType;

#include "Arc/Scene/Entity.h"

namespace ArcEngine
{
	class ScriptEngineRegistry
	{
	public:
		static void RegisterAll();

	private:
		static void InitComponentTypes();
		static void AddComponent(uint64_t entityID, void* type);
		static bool HasComponent(uint64_t entityID, void* type);

		static std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
		static std::unordered_map<MonoType*, std::function<void(Entity&)>> s_AddComponentFuncs;
	};
}
