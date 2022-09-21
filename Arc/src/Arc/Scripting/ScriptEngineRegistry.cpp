#include "arcpch.h"
#include "ScriptEngineRegistry.h"

#include <mono/jit/jit.h>
#include <glm/gtc/type_ptr.hpp>
#include <box2d/b2_body.h>

#include "Arc/Core/Input.h"
#include "Arc/Math/Math.h"
#include "Arc/Scene/Entity.h"
#include "GCManager.h"
#include "MonoUtils.h"
#include "ScriptEngine.h"

namespace ArcEngine
{
	eastl::hash_map<MonoType*, eastl::function<bool(const Entity&, MonoType*)>> ScriptEngineRegistry::s_HasComponentFuncs;
	eastl::hash_map<MonoType*, eastl::function<void(const Entity&, MonoType*)>> ScriptEngineRegistry::s_AddComponentFuncs;
	eastl::hash_map<MonoType*, eastl::function<GCHandle(const Entity&, MonoType*)>> ScriptEngineRegistry::s_GetComponentFuncs;

	template<typename... Component>
	void ScriptEngineRegistry::RegisterComponent()
	{
		([]()
		{
			constexpr size_t n = eastl::string_view("struct ArcEngine::").size();
			const char* componentName = n + typeid(Component).name();
			std::string name = std::string("ArcEngine.") + componentName;
			MonoType* type = mono_reflection_type_from_name(&name[0], ScriptEngine::GetCoreAssemblyImage());
			if (type)
			{
				ARC_CORE_TRACE("Registering {}", name.c_str());
				s_HasComponentFuncs[type] = [](const Entity& entity, [[maybe_unused]] MonoType*) { return entity.HasComponent<Component>(); };
				s_AddComponentFuncs[type] = [](const Entity& entity, [[maybe_unused]] MonoType*) { entity.AddComponent<Component>(); };
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
			s_HasComponentFuncs[type] = [](const Entity& entity, MonoType* monoType) { eastl::string scriptClassName = mono_type_get_name(monoType); return ScriptEngine::HasInstance(entity, scriptClassName); };
			s_AddComponentFuncs[type] = [](const Entity& entity, MonoType* monoType) { eastl::string scriptClassName = mono_type_get_name(monoType); ScriptEngine::CreateInstance(entity, scriptClassName); };
			s_GetComponentFuncs[type] = [](const Entity& entity, MonoType* monoType) { eastl::string scriptClassName = mono_type_get_name(monoType); return ScriptEngine::GetInstance(entity, scriptClassName)->GetHandle(); };
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
		ARC_PROFILE_SCOPE();

		const auto& scripts = ScriptEngine::GetClasses();
		for (const auto& [className, script] : scripts)
			RegisterScriptComponent(className);
	}

	void ScriptEngineRegistry::RegisterTypes()
	{
		ARC_PROFILE_SCOPE();

		InitComponentTypes();
		InitScriptComponentTypes();
	}

	void ScriptEngineRegistry::ClearTypes()
	{
		ARC_PROFILE_SCOPE();

		s_HasComponentFuncs.clear();
		s_GetComponentFuncs.clear();
		s_AddComponentFuncs.clear();
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Logging ////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	void Log_LogMessage(Log::Level level, MonoString* formattedMessage)
	{
		ARC_PROFILE_SCOPE();

		switch (level)
		{
			case Log::Level::Trace:		ARC_APP_TRACE(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Debug:		ARC_APP_DEBUG(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Info:		ARC_APP_INFO(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Warn:		ARC_APP_WARN(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Error:		ARC_APP_ERROR(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			case Log::Level::Critical:	ARC_APP_CRITICAL(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
			default:					ARC_APP_TRACE(MonoUtils::MonoStringToUTF8(formattedMessage));
										break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Input //////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	bool Input_IsKeyPressed(KeyCode key)
	{
		ARC_PROFILE_SCOPE();

		return Input::IsKeyPressed(key);
	}

	bool Input_IsMouseButtonPressed(MouseCode button)
	{
		ARC_PROFILE_SCOPE();

		return Input::IsMouseButtonPressed(button);
	}

	void Input_GetMousePosition(glm::vec2* outMousePosition)
	{
		ARC_PROFILE_SCOPE();

		*outMousePosition = Input::GetMousePosition();
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Entity /////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////
	Entity GetEntity(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(ScriptEngine::GetScene(), "Active scene is null");
		return ScriptEngine::GetScene()->GetEntity(entityID);
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
		ARC_CORE_ASSERT(s_HasComponentFuncs.find(monoType) != s_HasComponentFuncs.end(), mono_type_get_name(monoType));
		return s_HasComponentFuncs.at(monoType)(GetEntity(entityID), monoType);
	}

	MonoObject* ScriptEngineRegistry::GetComponent(uint64_t entityID, void* type)
	{
		ARC_PROFILE_SCOPE();

		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);

		auto it = s_GetComponentFuncs.find(monoType);
		if (it != s_GetComponentFuncs.end())
		{
			if (GCHandle handle = it->second(GetEntity(entityID), monoType))
				return GCManager::GetReferencedObject(handle);
		}

		return nullptr;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Transform //////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	void TransformComponent_GetTransform(uint64_t entityID, TransformComponent* outTransform)
	{
		ARC_PROFILE_SCOPE();

		*outTransform = GetEntity(entityID).GetComponent<TransformComponent>();
	}
	
	void TransformComponent_SetTransform(uint64_t entityID, const TransformComponent* inTransform)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<TransformComponent>() = *inTransform;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Tag ////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	MonoString* TagComponent_GetTag(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& tag = GetEntity(entityID).GetComponent<TagComponent>();
		return mono_string_new(mono_domain_get(), tag.Tag.c_str());
	}

	void TagComponent_SetTag(uint64_t entityID, MonoString* tag)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<TagComponent>().Tag = MonoUtils::MonoStringToUTF8(tag);
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Sprite Renderer ////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	void SpriteRendererComponent_GetColor(uint64_t entityID, glm::vec4* outTint)
	{
		ARC_PROFILE_SCOPE();

		*outTint = GetEntity(entityID).GetComponent<SpriteRendererComponent>().Color;
	}

	void SpriteRendererComponent_SetColor(uint64_t entityID, const glm::vec4* tint)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<SpriteRendererComponent>().Color = *tint;
	}

	void SpriteRendererComponent_GetTilingFactor(uint64_t entityID, float* outTiling)
	{
		ARC_PROFILE_SCOPE();

		*outTiling = GetEntity(entityID).GetComponent<SpriteRendererComponent>().TilingFactor;
	}

	void SpriteRendererComponent_SetTilingFactor(uint64_t entityID, const float* tiling)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<SpriteRendererComponent>().TilingFactor = *tiling;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Rigidbody 2D ///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	void Rigidbody2DComponent_GetBodyType(uint64_t entityID, int32_t* outType)
	{
		ARC_PROFILE_SCOPE();

		*outType = (int32_t)GetEntity(entityID).GetComponent<Rigidbody2DComponent>().Type;
	}

	void Rigidbody2DComponent_SetBodyType(uint64_t entityID, const int32_t* type)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.Type = (Rigidbody2DComponent::BodyType)(*type);
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetType((b2BodyType)(*type));
		}
	}

	void Rigidbody2DComponent_GetAutoMass(uint64_t entityID, bool* outAutoMass)
	{
		ARC_PROFILE_SCOPE();

		*outAutoMass = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().AutoMass;
	}

	void Rigidbody2DComponent_SetAutoMass(uint64_t entityID, const bool* autoMass)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.AutoMass = *autoMass;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			if (!component.AutoMass)
			{
				b2MassData massData = body->GetMassData();
				massData.mass = glm::max(component.Mass, 0.011f);
				body->SetMassData(&massData);
			}
			else
			{
				body->ResetMassData();
			}
		}
	}

	void Rigidbody2DComponent_GetMass(uint64_t entityID, float* outMass)
	{
		ARC_PROFILE_SCOPE();

		*outMass = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().Mass;
	}

	void Rigidbody2DComponent_SetMass(uint64_t entityID, const float* mass)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			if (!component.AutoMass)
			{
				component.Mass = glm::max(*mass, 0.011f);
				b2MassData massData = body->GetMassData();
				massData.mass = component.Mass;
				body->SetMassData(&massData);
			}
			else
			{
				body->ResetMassData();
			}
		}
	}

	void Rigidbody2DComponent_GetLinearDrag(uint64_t entityID, float* outDrag)
	{
		ARC_PROFILE_SCOPE();

		*outDrag = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().LinearDrag;
	}

	void Rigidbody2DComponent_SetLinearDrag(uint64_t entityID, const float* drag)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.LinearDrag = glm::max(*drag, 0.0f);
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetLinearDamping(component.LinearDrag);
		}
	}

	void Rigidbody2DComponent_GetAngularDrag(uint64_t entityID, float* outDrag)
	{
		ARC_PROFILE_SCOPE();

		*outDrag = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().AngularDrag;
	}

	void Rigidbody2DComponent_SetAngularDrag(uint64_t entityID, const float* drag)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.AngularDrag = glm::max(*drag, 0.0f);
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAngularDamping(component.AngularDrag);
		}
	}

	void Rigidbody2DComponent_GetAllowSleep(uint64_t entityID, bool* outState)
	{
		ARC_PROFILE_SCOPE();

		*outState = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().AllowSleep;
	}

	void Rigidbody2DComponent_SetAllowSleep(uint64_t entityID, const bool* state)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.AllowSleep = *state;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetSleepingAllowed(component.AllowSleep);
		}
	}

	void Rigidbody2DComponent_GetAwake(uint64_t entityID, bool* outState)
	{
		ARC_PROFILE_SCOPE();

		*outState = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().Awake;
	}

	void Rigidbody2DComponent_SetAwake(uint64_t entityID, const bool* state)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.Awake = *state;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAwake(component.Awake);
		}
	}

	void Rigidbody2DComponent_GetContinuous(uint64_t entityID, bool* outState)
	{
		ARC_PROFILE_SCOPE();

		*outState = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().Continuous;
	}

	void Rigidbody2DComponent_SetContinuous(uint64_t entityID, const bool* state)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.Continuous = *state;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetBullet(component.Continuous);
		}
	}

	void Rigidbody2DComponent_GetFreezeRotation(uint64_t entityID, bool* outState)
	{
		ARC_PROFILE_SCOPE();

		*outState = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().FreezeRotation;
	}

	void Rigidbody2DComponent_SetFreezeRotation(uint64_t entityID, const bool* state)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.FreezeRotation = *state;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetFixedRotation(component.FreezeRotation);
		}
	}

	void Rigidbody2DComponent_GetGravityScale(uint64_t entityID, float* outGravityScale)
	{
		ARC_PROFILE_SCOPE();

		*outGravityScale = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().GravityScale;
	}

	void Rigidbody2DComponent_SetGravityScale(uint64_t entityID, const bool* gravityScale)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.GravityScale = *gravityScale;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetGravityScale(component.GravityScale);
		}
	}

	void Rigidbody2DComponent_ApplyForceAtCenter(uint64_t entityID, const glm::vec2* force)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->ApplyForceToCenter({ force->x, force->y }, true);
		}
	}

	void Rigidbody2DComponent_ApplyForce(uint64_t entityID, const glm::vec2* force, const glm::vec2* point)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->ApplyForce({ force->x, force->y }, { point->x, point->y }, true);
		}
	}

	void Rigidbody2DComponent_ApplyLinearImpulse(uint64_t entityID, const glm::vec2* impulse, const glm::vec2* point)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->ApplyLinearImpulse({ impulse->x, impulse->y }, { point->x, point->y }, true);
		}
	}

	void Rigidbody2DComponent_ApplyLinearImpulseAtCenter(uint64_t entityID, const glm::vec2* impulse)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->ApplyLinearImpulseToCenter({ impulse->x, impulse->y }, true);
		}
	}

	void Rigidbody2DComponent_ApplyAngularImpulse(uint64_t entityID, const float* impulse)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->ApplyAngularImpulse(*impulse, true);
		}
	}

	void Rigidbody2DComponent_ApplyTorque(uint64_t entityID, const float* torque)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->ApplyTorque(*torque, true);
		}
	}

	void Rigidbody2DComponent_IsAwake(uint64_t entityID, bool* outAwake)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			const b2Body* body = (b2Body*)component.RuntimeBody;
			*outAwake = body->IsAwake();
		}
		else
		{
			*outAwake = false;
		}
	}

	void Rigidbody2DComponent_IsSleeping(uint64_t entityID, bool* outSleeping)
	{
		ARC_PROFILE_SCOPE();

		bool awake;
		Rigidbody2DComponent_IsAwake(entityID, &awake);
		*outSleeping = !awake;
	}

	void Rigidbody2DComponent_MovePosition(uint64_t entityID, const glm::vec2* position)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetTransform({ position->x, position->y }, body->GetAngle());
		}
	}

	void Rigidbody2DComponent_MoveRotation(uint64_t entityID, const float* angle)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetTransform(body->GetPosition(), glm::radians(*angle));
		}
	}

	void Rigidbody2DComponent_GetVelocity(uint64_t entityID, glm::vec2* outVelocity)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			const b2Body* body = (b2Body*)component.RuntimeBody;
			b2Vec2 velocity = body->GetLinearVelocity();
			outVelocity->x = velocity.x;
			outVelocity->y = velocity.y;
		}
		else
		{
			*outVelocity = glm::vec2(0.0f);
		}
	}

	void Rigidbody2DComponent_SetVelocity(uint64_t entityID, const glm::vec2* velocity)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetLinearVelocity({ velocity->x, velocity->y });
		}
	}

	void Rigidbody2DComponent_GetAngularVelocity(uint64_t entityID, float* outVelocity)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			const b2Body* body = (b2Body*)component.RuntimeBody;
			*outVelocity = body->GetAngularVelocity();
		}
		else
		{
			*outVelocity = 0.0f;
		}
	}

	void Rigidbody2DComponent_SetAngularVelocity(uint64_t entityID, const float* velocity)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAngularVelocity(*velocity);
		}
	}

	void Rigidbody2DComponent_Sleep(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAwake(false);
		}
	}

	void Rigidbody2DComponent_WakeUp(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAwake(true);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Audio Source ////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	void AudioSourceComponent_GetVolume(uint64_t entityID, float* outVolume)
	{
		ARC_PROFILE_SCOPE();

		*outVolume = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.VolumeMultiplier;
	}

	void AudioSourceComponent_SetVolume(uint64_t entityID, const float* volume)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.VolumeMultiplier = *volume;
		if (component.Source)
			component.Source->SetVolume(*volume);
	}

	void AudioSourceComponent_GetPitch(uint64_t entityID, float* outPitch)
	{
		ARC_PROFILE_SCOPE();

		*outPitch = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.PitchMultiplier;
	}

	void AudioSourceComponent_SetPitch(uint64_t entityID, const float* pitch)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.PitchMultiplier = *pitch;
		if (component.Source)
			component.Source->SetVolume(*pitch);
	}

	void AudioSourceComponent_GetPlayOnAwake(uint64_t entityID, bool* outPlayOnAwake)
	{
		ARC_PROFILE_SCOPE();

		*outPlayOnAwake = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.PlayOnAwake;
	}

	void AudioSourceComponent_SetPlayOnAwake(uint64_t entityID, const bool* playOnAwake)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.PlayOnAwake = *playOnAwake;
	}

	void AudioSourceComponent_GetLooping(uint64_t entityID, bool* outLooping)
	{
		ARC_PROFILE_SCOPE();

		*outLooping = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.Looping;
	}

	void AudioSourceComponent_SetLooping(uint64_t entityID, const bool* looping)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.Looping = *looping;
		if (component.Source)
			component.Source->SetLooping(*looping);
	}

	void AudioSourceComponent_GetSpatialization(uint64_t entityID, bool* outSpatialization)
	{
		ARC_PROFILE_SCOPE();

		*outSpatialization = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.Spatialization;
	}

	void AudioSourceComponent_SetSpatialization(uint64_t entityID, const bool* spatialization)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.Spatialization = *spatialization;
		if (component.Source)
			component.Source->SetSpatialization(*spatialization);
	}

	void AudioSourceComponent_GetAttenuationModel(uint64_t entityID, int* outAttenuationModel)
	{
		ARC_PROFILE_SCOPE();

		*outAttenuationModel = (int) GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.AttenuationModel;
	}

	void AudioSourceComponent_SetAttenuationModel(uint64_t entityID, const int* attenuationModel)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.AttenuationModel = (AttenuationModelType)(*attenuationModel);
		if (component.Source)
			component.Source->SetAttenuationModel(component.Config.AttenuationModel);
	}

	void AudioSourceComponent_GetRollOff(uint64_t entityID, float* outRollOff)
	{
		ARC_PROFILE_SCOPE();

		*outRollOff = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.RollOff;
	}

	void AudioSourceComponent_SetRollOff(uint64_t entityID, const float* rollOff)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.RollOff = *rollOff;
		if (component.Source)
			component.Source->SetRollOff(*rollOff);
	}

	void AudioSourceComponent_GetMinGain(uint64_t entityID, float* outMinGain)
	{
		ARC_PROFILE_SCOPE();

		*outMinGain = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MinGain;
	}

	void AudioSourceComponent_SetMinGain(uint64_t entityID, const float* minGain)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MinGain = *minGain;
		if (component.Source)
			component.Source->SetMinGain(*minGain);
	}

	void AudioSourceComponent_GetMaxGain(uint64_t entityID, float* outMaxGain)
	{
		ARC_PROFILE_SCOPE();

		*outMaxGain = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MaxGain;
	}

	void AudioSourceComponent_SetMaxGain(uint64_t entityID, const float* maxGain)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MaxGain = *maxGain;
		if (component.Source)
			component.Source->SetMaxGain(*maxGain);
	}

	void AudioSourceComponent_GetMinDistance(uint64_t entityID, float* outMinDistance)
	{
		ARC_PROFILE_SCOPE();

		*outMinDistance = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MinDistance;
	}

	void AudioSourceComponent_SetMinDistance(uint64_t entityID, const float* minDistance)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MinDistance = *minDistance;
		if (component.Source)
			component.Source->SetMinDistance(*minDistance);
	}

	void AudioSourceComponent_GetMaxDistance(uint64_t entityID, float* outMaxDistance)
	{
		ARC_PROFILE_SCOPE();

		*outMaxDistance = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MaxDistance;
	}

	void AudioSourceComponent_SetMaxDistance(uint64_t entityID, const float* maxDistance)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MaxDistance = *maxDistance;
		if (component.Source)
			component.Source->SetMaxDistance(*maxDistance);
	}

	void AudioSourceComponent_GetConeInnerAngle(uint64_t entityID, float* outConeInnerAngle)
	{
		ARC_PROFILE_SCOPE();

		*outConeInnerAngle = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.ConeInnerAngle;
	}

	void AudioSourceComponent_SetConeInnerAngle(uint64_t entityID, const float* coneInnerAngle)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeInnerAngle = *coneInnerAngle;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	void AudioSourceComponent_GetConeOuterAngle(uint64_t entityID, float* outConeOuterAngle)
	{
		ARC_PROFILE_SCOPE();

		*outConeOuterAngle = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.ConeOuterAngle;
	}

	void AudioSourceComponent_SetConeOuterAngle(uint64_t entityID, const float* coneOuterAngle)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeOuterAngle = *coneOuterAngle;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	void AudioSourceComponent_GetConeOuterGain(uint64_t entityID, float* outConeOuterGain)
	{
		ARC_PROFILE_SCOPE();

		*outConeOuterGain = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.ConeOuterGain;
	}

	void AudioSourceComponent_SetConeOuterGain(uint64_t entityID, const float* coneOuterGain)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeOuterGain = *coneOuterGain;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	void AudioSourceComponent_SetCone(uint64_t entityID, const float* coneInnerAngle, const float* coneOuterAngle, const float* coneOuterGain)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeInnerAngle = *coneInnerAngle;
		component.Config.ConeOuterAngle = *coneOuterAngle;
		component.Config.ConeOuterGain = *coneOuterGain;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	void AudioSourceComponent_GetDopplerFactor(uint64_t entityID, float* outDopplerFactor)
	{
		ARC_PROFILE_SCOPE();

		*outDopplerFactor = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.DopplerFactor;
	}

	void AudioSourceComponent_SetDopplerFactor(uint64_t entityID, const float* dopplerFactor)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.DopplerFactor = *dopplerFactor;
		if (component.Source)
			component.Source->SetDopplerFactor(*dopplerFactor);
	}

	void AudioSourceComponent_IsPlaying(uint64_t entityID, bool* outIsPlaying)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			*outIsPlaying = component.Source->IsPlaying();
		else
			*outIsPlaying = false;
	}

	void AudioSourceComponent_Play(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->Play();
	}

	void AudioSourceComponent_Pause(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->Pause();
	}

	void AudioSourceComponent_UnPause(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->UnPause();
	}

	void AudioSourceComponent_Stop(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->Stop();
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

#define ARC_ADD_INTERNAL_CALL(Name) mono_add_internal_call("ArcEngine.InternalCalls::" #Name, &Name)

	void ScriptEngineRegistry::RegisterInternalCalls()
	{
		ARC_PROFILE_SCOPE();

		///////////////////////////////////////////////////////////////
		// Entity /////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::Entity_AddComponent", &ScriptEngineRegistry::AddComponent);
		mono_add_internal_call("ArcEngine.InternalCalls::Entity_HasComponent", &ScriptEngineRegistry::HasComponent);
		mono_add_internal_call("ArcEngine.InternalCalls::Entity_GetComponent", &ScriptEngineRegistry::GetComponent);

		///////////////////////////////////////////////////////////////
		// Tag ////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		ARC_ADD_INTERNAL_CALL(TagComponent_GetTag);
		ARC_ADD_INTERNAL_CALL(TagComponent_SetTag);

		///////////////////////////////////////////////////////////////
		// Transform //////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		ARC_ADD_INTERNAL_CALL(TransformComponent_GetTransform);
		ARC_ADD_INTERNAL_CALL(TransformComponent_SetTransform);

		///////////////////////////////////////////////////////////////
		// Input //////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		ARC_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		ARC_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		ARC_ADD_INTERNAL_CALL(Input_GetMousePosition);

		///////////////////////////////////////////////////////////////
		// Logging ////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		ARC_ADD_INTERNAL_CALL(Log_LogMessage);

		///////////////////////////////////////////////////////////////
		// SpriteRenderer /////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		ARC_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		ARC_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);
		ARC_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTilingFactor);
		ARC_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTilingFactor);

		///////////////////////////////////////////////////////////////
		// Rigidbody 2D ///////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetBodyType);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetBodyType);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetAutoMass);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetAutoMass);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetMass);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetMass);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearDrag);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetLinearDrag);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetAngularDrag);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetAngularDrag);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetAllowSleep);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetAllowSleep);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetAwake);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetAwake);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetContinuous);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetContinuous);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetFreezeRotation);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetFreezeRotation);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetGravityScale);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetGravityScale);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyForceAtCenter);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyForce);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseAtCenter);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyAngularImpulse);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyTorque);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_IsAwake);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_IsSleeping);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_MovePosition);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_MoveRotation);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetVelocity);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetVelocity);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetAngularVelocity);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetAngularVelocity);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_Sleep);
		ARC_ADD_INTERNAL_CALL(Rigidbody2DComponent_WakeUp);

		///////////////////////////////////////////////////////////////
		// Audio Source ///////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetVolume);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetVolume);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetPitch);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetPitch);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetPlayOnAwake);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetPlayOnAwake);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetLooping);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetLooping);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetSpatialization);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetSpatialization);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetAttenuationModel);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetAttenuationModel);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetRollOff);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetRollOff);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetMinGain);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetMinGain);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetMaxGain);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetMaxGain);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetMinDistance);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetMinDistance);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetMaxDistance);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetMaxDistance);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetConeInnerAngle);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetConeInnerAngle);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetConeOuterAngle);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetConeOuterAngle);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetConeOuterGain);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetConeOuterGain);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetCone);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_GetDopplerFactor);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_SetDopplerFactor);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_IsPlaying);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_Play);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_Pause);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_UnPause);
		ARC_ADD_INTERNAL_CALL(AudioSourceComponent_Stop);
	}
}
