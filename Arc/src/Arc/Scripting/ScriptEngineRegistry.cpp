#include "arcpch.h"
#include "ScriptEngineRegistry.h"

#include <mono/jit/jit.h>
#include <glm/gtc/type_ptr.hpp>

#include "ScriptEngine.h"
#include "MonoUtils.h";
#include "Arc/Core/Input.h"
#include "Arc/Math/Math.h"

namespace ArcEngine
{
	eastl::hash_map<MonoType*, eastl::function<bool(Entity&)>> ScriptEngineRegistry::s_HasComponentFuncs;
	eastl::hash_map<MonoType*, eastl::function<void(Entity&)>> ScriptEngineRegistry::s_AddComponentFuncs;
	
	#define COMPONENT_REGISTER(Type)\
	{\
		MonoType* type = mono_reflection_type_from_name("ArcEngine." #Type, ScriptEngine::GetCoreAssemblyImage());\
		if (type)\
		{\
			uint32_t id = mono_type_get_type(type);\
			s_HasComponentFuncs[type] = [](Entity& entity){ return entity.HasComponent<Type>(); };\
			s_AddComponentFuncs[type] = [](Entity& entity){ entity.AddComponent<Type>(); };\
		}\
	}

	void ScriptEngineRegistry::InitComponentTypes()
	{
		ARC_PROFILE_SCOPE();

		COMPONENT_REGISTER(TagComponent);
		COMPONENT_REGISTER(TransformComponent);
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
		s_AddComponentFuncs.at(monoType)(GetEntity(entityID));
	}

	bool ScriptEngineRegistry::HasComponent(uint64_t entityID, void* type)
	{
		ARC_PROFILE_SCOPE();

		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		return s_HasComponentFuncs.at(monoType)(GetEntity(entityID));
	}

	void ScriptEngineRegistry::RegisterAll()
	{
		ARC_PROFILE_SCOPE();

		InitComponentTypes();

		///////////////////////////////////////////////////////////////
		// Entity /////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::Entity_AddComponent", ScriptEngineRegistry::AddComponent);
		mono_add_internal_call("ArcEngine.InternalCalls::Entity_HasComponent", ScriptEngineRegistry::HasComponent);

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
