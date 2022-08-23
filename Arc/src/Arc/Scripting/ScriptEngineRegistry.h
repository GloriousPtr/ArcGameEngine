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

		template<typename... Component>
		static void RegisterComponent();

		template<typename... Component>
		static void RegisterComponent(ComponentGroup<Component...>);

		static eastl::hash_map<MonoType*, eastl::function<bool(Entity&)>> s_HasComponentFuncs;
		static eastl::hash_map<MonoType*, eastl::function<void(Entity&)>> s_AddComponentFuncs;
	};
}
