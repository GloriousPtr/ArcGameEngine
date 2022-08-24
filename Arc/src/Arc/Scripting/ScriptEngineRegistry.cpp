#include "arcpch.h"
#include "ScriptEngineRegistry.h"

#include <mono/jit/jit.h>
#include <glm/gtc/type_ptr.hpp>

#include "ScriptEngine.h"
#include "GCManager.h"
#include "MonoUtils.h";
#include "Arc/Core/Input.h"
#include "Arc/Math/Math.h"

namespace ArcEngine
{
	eastl::hash_map<MonoType*, eastl::function<bool(Entity&, MonoType*)>> ScriptEngineRegistry::s_HasComponentFuncs;
	eastl::hash_map<MonoType*, eastl::function<void(Entity&, MonoType*)>> ScriptEngineRegistry::s_AddComponentFuncs;
	eastl::hash_map<MonoType*, eastl::function<GCHandle(Entity&, MonoType*)>> ScriptEngineRegistry::s_GetComponentFuncs;

	template<typename... Component>
	void ScriptEngineRegistry::RegisterComponent()
	{
		([]()
		{
			static int n = strlen("struct ArcEngine::");
			const char* componentName = n + typeid(Component).name();
			std::string name = std::string("ArcEngine.") + componentName;
			MonoType* type = mono_reflection_type_from_name(&name[0], ScriptEngine::GetCoreAssemblyImage());
			if (type)
			{
				ARC_CORE_TRACE("Registering {}", name.c_str());
				uint32_t id = mono_type_get_type(type);
				s_HasComponentFuncs[type] = [](Entity& entity, MonoType* monoType) { return entity.HasComponent<Component>(); };
				s_AddComponentFuncs[type] = [](Entity& entity, MonoType* monoType) { entity.AddComponent<Component>(); };
			}
			else
			{
				//ARC_CORE_ERROR("Component not found: {}", name.c_str());
			}
		}(), ...);
	}

	template<typename... Component>
	void ScriptEngineRegistry::RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptEngineRegistry::RegisterScriptComponent(const eastl::string& className)
	{
		eastl::string name = className;
		MonoType* type = mono_reflection_type_from_name(&name[0], ScriptEngine::GetAppAssemblyImage());
		if (type)
		{
			ARC_CORE_TRACE("Registering {}", name.c_str());
			uint32_t id = mono_type_get_type(type);
			s_HasComponentFuncs[type] = [](Entity& entity, MonoType* monoType) { eastl::string className = mono_type_get_name(monoType); return ScriptEngine::HasInstance(entity, className); };
			s_AddComponentFuncs[type] = [](Entity& entity, MonoType* monoType) { eastl::string className = mono_type_get_name(monoType); ScriptEngine::CreateInstance(entity, className); };
			s_GetComponentFuncs[type] = [](Entity& entity, MonoType* monoType) { eastl::string className = mono_type_get_name(monoType); return ScriptEngine::GetInstance(entity, className)->GetHandle(); };
		}
	}

	void ScriptEngineRegistry::InitComponentTypes()
	{
		ARC_PROFILE_SCOPE();

		RegisterComponent<TagComponent>();
		RegisterComponent<TransformComponent>();
		RegisterComponent(AllComponents{});
	}

	void ScriptEngineRegistry::InitScriptComponentTypes()
	{
		auto& scripts = ScriptEngine::GetClasses();
		for (auto [className, script] : scripts)
			RegisterScriptComponent(className.c_str());
	}

	void LogMessage(Log::Level level, MonoString* formattedMessage)
	{
		ARC_PROFILE_SCOPE();

		switch (level)
		{
			case Log::Level::Trace:		ARC_TRACE(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Debug:		ARC_DEBUG(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Info:		ARC_INFO(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Warn:		ARC_WARN(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Error:		ARC_ERROR(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Critical:	ARC_CRITICAL(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
		}
	}

	Entity GetEntity(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(ScriptEngine::GetScene(), "Active scene is null");
		return ScriptEngine::GetScene()->GetEntity(entityID);
	}

	void GetTransform(uint64_t entityID, TransformComponent* outTransform)
	{
		ARC_PROFILE_SCOPE();

		*outTransform = GetEntity(entityID).GetComponent<TransformComponent>();
	}
	
	void SetTransform(uint64_t entityID, TransformComponent* inTransform)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<TransformComponent>() = *inTransform;
	}

	void GetMousePosition(glm::vec2* outMousePosition)
	{
		ARC_PROFILE_SCOPE();

		*outMousePosition = Input::GetMousePosition();
	}

	MonoString* GetTag(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		auto& tag = GetEntity(entityID).GetComponent<TagComponent>();
		return mono_string_new(mono_domain_get(), tag.Tag.c_str());
	}

	void SetTag(uint64_t entityID, MonoString* tag)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<TagComponent>().Tag = MonoUtils::MonoStringToUTF8(tag);
	}

	void ScriptEngineRegistry::AddComponent(uint64_t entityID, void* type)
	{
		ARC_PROFILE_SCOPE();

		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		ARC_CORE_ASSERT(s_AddComponentFuncs.find(monoType) != s_AddComponentFuncs.end(), mono_type_get_name(monoType));
		s_AddComponentFuncs.at(monoType)(GetEntity(entityID), monoType);
	}

	bool ScriptEngineRegistry::HasComponent(uint64_t entityID, void* type)
	{
		ARC_PROFILE_SCOPE();

		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		eastl::string n = mono_type_get_name(monoType);
		ARC_CORE_ERROR(n.c_str());
		ARC_CORE_ASSERT(s_HasComponentFuncs.find(monoType) != s_HasComponentFuncs.end(), mono_type_get_name(monoType));
		return s_HasComponentFuncs.at(monoType)(GetEntity(entityID), monoType);
	}

	void ScriptEngineRegistry::GetComponent(uint64_t entityID, void* type, GCHandle* outHandle)
	{
		ARC_PROFILE_SCOPE();

		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		eastl::string n = mono_type_get_name(monoType);
		ARC_CORE_ERROR(n.c_str());
		ARC_CORE_ASSERT(s_GetComponentFuncs.find(monoType) != s_GetComponentFuncs.end(), mono_type_get_name(monoType));
		*outHandle = s_GetComponentFuncs.at(monoType)(GetEntity(entityID), monoType);
	}

	void ScriptEngineRegistry::RegisterTypes()
	{
		InitComponentTypes();
		InitScriptComponentTypes();
	}

	void ScriptEngineRegistry::ClearTypes()
	{
		s_HasComponentFuncs.clear();
		s_GetComponentFuncs.clear();
		s_AddComponentFuncs.clear();
	}

	void ScriptEngineRegistry::RegisterInternalCalls()
	{
		ARC_PROFILE_SCOPE();

		///////////////////////////////////////////////////////////////
		// Entity /////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::Entity_AddComponent", ScriptEngineRegistry::AddComponent);
		mono_add_internal_call("ArcEngine.InternalCalls::Entity_HasComponent", ScriptEngineRegistry::HasComponent);
		mono_add_internal_call("ArcEngine.InternalCalls::Entity_GetComponent", ScriptEngineRegistry::GetComponent);

		///////////////////////////////////////////////////////////////
		// Tag ////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::TagComponent_GetTag", GetTag);
		mono_add_internal_call("ArcEngine.InternalCalls::TagComponent_SetTag", SetTag);

		///////////////////////////////////////////////////////////////
		// Transform //////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::TransformComponent_GetTransform", GetTransform);
		mono_add_internal_call("ArcEngine.InternalCalls::TransformComponent_SetTransform", SetTransform);

		///////////////////////////////////////////////////////////////
		// Input //////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::Input_IsKeyPressed", Input::IsKeyPressed);
		mono_add_internal_call("ArcEngine.InternalCalls::Input_IsMouseButtonPressed", Input::IsMouseButtonPressed);
		mono_add_internal_call("ArcEngine.InternalCalls::Input_GetMousePosition", GetMousePosition);

		///////////////////////////////////////////////////////////////
		// Logging ////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::Log_LogMessage", LogMessage);
	}
}
