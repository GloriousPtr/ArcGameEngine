#pragma once

#include "Arc/Scene/Components.h"

typedef struct _MonoType MonoType;
typedef struct _MonoObject MonoObject;

namespace ArcEngine
{
	class ScriptEngineRegistry
	{
	public:
		static void RegisterInternalCalls();
		static void RegisterTypes();
		static void ClearTypes();

	private:
		static void InitComponentTypes();
		static void InitScriptComponentTypes();
		static void AddComponent(uint64_t entityID, void* type);
		static bool HasComponent(uint64_t entityID, void* type);
		static void GetComponent(uint64_t entityID, void* type, GCHandle* outHandle);

		template<typename... Component>
		static void RegisterComponent();
		template<typename... Component>
		static void RegisterComponent(ComponentGroup<Component...>);
		static void RegisterScriptComponent(const eastl::string& className);

		static eastl::hash_map<MonoType*, eastl::function<bool(const Entity&, MonoType*)>> s_HasComponentFuncs;
		static eastl::hash_map<MonoType*, eastl::function<void(const Entity&, MonoType*)>> s_AddComponentFuncs;
		static eastl::hash_map<MonoType*, eastl::function<GCHandle(const Entity&, MonoType*)>> s_GetComponentFuncs;
	};
}
