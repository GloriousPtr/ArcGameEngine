#pragma once

typedef struct _MonoType MonoType;
typedef struct _MonoObject MonoObject;

namespace ArcEngine
{
	class Entity;
	template<typename... Component>
	struct ComponentGroup;

	using GCHandle = uint32_t;

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
		[[nodiscard]] static MonoObject* GetComponent(uint64_t entityID, void* type);

		template<typename... Component>
		static void RegisterComponent();
		template<typename... Component>
		static void RegisterComponent(ComponentGroup<Component...>);
		static void RegisterScriptComponent(const std::string& className);

		static std::unordered_map<MonoType*, std::function<bool(const Entity&, MonoType*)>> s_HasComponentFuncs;
		static std::unordered_map<MonoType*, std::function<void(const Entity&, MonoType*)>> s_AddComponentFuncs;
		static std::unordered_map<MonoType*, std::function<GCHandle(const Entity&, MonoType*)>> s_GetComponentFuncs;
	};
}
