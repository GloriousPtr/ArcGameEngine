#include "arcpch.h"
#include "ScriptEngineRegistry.h"

#include <mono/jit/jit.h>
#include <glm/gtc/type_ptr.hpp>
#include <box2d/b2_body.h>

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
				s_AddComponentFuncs[type] = [](Entity& entity, MonoType* monoType) {
					entity.AddComponent<Component>();
				};
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

	///////////////////////////////////////////////////////////////////////////////////////////
	// Logging ////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

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

	///////////////////////////////////////////////////////////////////////////////////////////
	// Input //////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	void GetMousePosition(glm::vec2* outMousePosition)
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

	void ScriptEngineRegistry::GetComponent(uint64_t entityID, void* type, GCHandle* outHandle)
	{
		ARC_PROFILE_SCOPE();

		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		ARC_CORE_ASSERT(s_GetComponentFuncs.find(monoType) != s_GetComponentFuncs.end(), mono_type_get_name(monoType));
		*outHandle = s_GetComponentFuncs.at(monoType)(GetEntity(entityID), monoType);
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Transform //////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

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

	///////////////////////////////////////////////////////////////////////////////////////////
	// Tag ////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

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

	///////////////////////////////////////////////////////////////////////////////////////////
	// Sprite Renderer ////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	void SpriteRenderer_GetColor(uint64_t entityID, glm::vec4* outTint)
	{
		*outTint = GetEntity(entityID).GetComponent<SpriteRendererComponent>().Color;
	}

	void SpriteRenderer_SetColor(uint64_t entityID, glm::vec4* tint)
	{
		GetEntity(entityID).GetComponent<SpriteRendererComponent>().Color = *tint;
	}

	void SpriteRenderer_GetTilingFactor(uint64_t entityID, float* outTiling)
	{
		*outTiling = GetEntity(entityID).GetComponent<SpriteRendererComponent>().TilingFactor;
	}

	void SpriteRenderer_SetTilingFactor(uint64_t entityID, float* tiling)
	{
		GetEntity(entityID).GetComponent<SpriteRendererComponent>().TilingFactor = *tiling;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Rigidbody 2D ///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	void Rigidbody2D_GetBodyType(uint64_t entityID, int32_t* outType)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outType = (int32_t)component.Type;
	}

	void Rigidbody2D_SetBodyType(uint64_t entityID, int32_t* type)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.Type = (Rigidbody2DComponent::BodyType)(*type);
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetType((b2BodyType)(*type));
		}
	}

	void Rigidbody2D_GetAutoMass(uint64_t entityID, bool* outAutoMass)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outAutoMass = component.AutoMass;
	}

	void Rigidbody2D_SetAutoMass(uint64_t entityID, bool* autoMass)
	{
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

	void Rigidbody2D_GetMass(uint64_t entityID, float* outMass)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outMass = component.Mass;
	}

	void Rigidbody2D_SetMass(uint64_t entityID, float* mass)
	{
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

	void Rigidbody2D_GetLinearDrag(uint64_t entityID, float* outDrag)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outDrag = component.LinearDrag;
	}

	void Rigidbody2D_SetLinearDrag(uint64_t entityID, float* drag)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.LinearDrag = glm::max(*drag, 0.0f);
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetLinearDamping(component.LinearDrag);
		}
	}

	void Rigidbody2D_GetAngularDrag(uint64_t entityID, float* outDrag)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outDrag = component.AngularDrag;
	}

	void Rigidbody2D_SetAngularDrag(uint64_t entityID, float* drag)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.AngularDrag = glm::max(*drag, 0.0f);
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAngularDamping(component.AngularDrag);
		}
	}

	void Rigidbody2D_GetAllowSleep(uint64_t entityID, bool* outState)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outState = component.AllowSleep;
	}

	void Rigidbody2D_SetAllowSleep(uint64_t entityID, bool* state)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.AllowSleep = *state;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetSleepingAllowed(component.AllowSleep);
		}
	}

	void Rigidbody2D_GetAwake(uint64_t entityID, bool* outState)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outState = component.Awake;
	}

	void Rigidbody2D_SetAwake(uint64_t entityID, bool* state)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.Awake = *state;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAwake(component.Awake);
		}
	}

	void Rigidbody2D_GetContinuous(uint64_t entityID, bool* outState)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outState = component.Continuous;
	}

	void Rigidbody2D_SetContinuous(uint64_t entityID, bool* state)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.Continuous = *state;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetBullet(component.Continuous);
		}
	}

	void Rigidbody2D_GetFreezeRotation(uint64_t entityID, bool* outState)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outState = component.FreezeRotation;
	}

	void Rigidbody2D_SetFreezeRotation(uint64_t entityID, bool* state)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.FreezeRotation = *state;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetFixedRotation(component.FreezeRotation);
		}
	}

	void Rigidbody2D_GetGravityScale(uint64_t entityID, float* outGravityScale)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		*outGravityScale = component.GravityScale;
	}

	void Rigidbody2D_SetGravityScale(uint64_t entityID, bool* gravityScale)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.GravityScale = *gravityScale;
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetGravityScale(component.GravityScale);
		}
	}

	void Rigidbody2D_ApplyForceAtCenter(uint64_t entityID, glm::vec2* force)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			glm::vec2& f = *force;
			body->ApplyForceToCenter({ f.x, f.y }, true);
		}
	}

	void Rigidbody2D_ApplyForce(uint64_t entityID, glm::vec2* force, glm::vec2* point)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			glm::vec2& f = *force;
			glm::vec2& p = *point;
			body->ApplyForce({ f.x, f.y }, { p.x, p.y }, true);
		}
	}

	void Rigidbody2D_ApplyLinearImpulse(uint64_t entityID, glm::vec2* impulse, glm::vec2* point)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			glm::vec2& i = *impulse;
			glm::vec2& p = *point;
			body->ApplyLinearImpulse({ i.x, i.y }, { p.x, p.y }, true);
		}
	}

	void Rigidbody2D_ApplyLinearImpulseAtCenter(uint64_t entityID, glm::vec2* impulse)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			glm::vec2& i = *impulse;
			body->ApplyLinearImpulseToCenter({ i.x, i.y }, true);
		}
	}

	void Rigidbody2D_ApplyAngularImpulse(uint64_t entityID, float* impulse)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->ApplyAngularImpulse(*impulse, true);
		}
	}

	void Rigidbody2D_ApplyTorque(uint64_t entityID, float* torque)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->ApplyTorque(*torque, true);
		}
	}

	void Rigidbody2D_IsAwake(uint64_t entityID, bool* outAwake)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			*outAwake = body->IsAwake();
		}
		else
		{
			*outAwake = false;
		}
	}

	void Rigidbody2D_IsSleeping(uint64_t entityID, bool* outSleeping)
	{
		bool awake;
		Rigidbody2D_IsAwake(entityID, &awake);
		*outSleeping = !awake;
	}

	void Rigidbody2D_MovePosition(uint64_t entityID, glm::vec2* position)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetTransform({ position->x, position->y }, body->GetAngle());
		}
	}

	void Rigidbody2D_MoveRotation(uint64_t entityID, float* angle)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetTransform(body->GetPosition(), glm::radians(*angle));
		}
	}

	void Rigidbody2D_GetVelocity(uint64_t entityID, glm::vec2* outVelocity)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			b2Vec2 velocity = body->GetLinearVelocity();
			outVelocity->x = velocity.x;
			outVelocity->y = velocity.y;
		}
		else
		{
			*outVelocity = glm::vec2(0.0f);
		}
	}

	void Rigidbody2D_SetVelocity(uint64_t entityID, glm::vec2* velocity)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetLinearVelocity({ velocity->x, velocity->y });
		}
	}

	void Rigidbody2D_GetAngularVelocity(uint64_t entityID, float* outVelocity)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			*outVelocity = body->GetAngularVelocity();
		}
		else
		{
			*outVelocity = 0.0f;
		}
	}

	void Rigidbody2D_SetAngularVelocity(uint64_t entityID, float* velocity)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAngularVelocity(*velocity);
		}
	}

	void Rigidbody2D_Sleep(uint64_t entityID)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAwake(false);
		}
	}

	void Rigidbody2D_WakeUp(uint64_t entityID)
	{
		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (component.RuntimeBody)
		{
			b2Body* body = (b2Body*)component.RuntimeBody;
			body->SetAwake(true);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Audio Source ////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	void AudioSource_GetVolume(uint64_t entityID, float* outVolume)
	{
		*outVolume = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.VolumeMultiplier;
	}

	void AudioSource_SetVolume(uint64_t entityID, float* volume)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.VolumeMultiplier = *volume;
		if (component.Source)
			component.Source->SetVolume(*volume);
	}

	void AudioSource_GetPitch(uint64_t entityID, float* outPitch)
	{
		*outPitch = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.PitchMultiplier;
	}

	void AudioSource_SetPitch(uint64_t entityID, float* pitch)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.PitchMultiplier = *pitch;
		if (component.Source)
			component.Source->SetVolume(*pitch);
	}

	void AudioSource_GetPlayOnAwake(uint64_t entityID, bool* outPlayOnAwake)
	{
		*outPlayOnAwake = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.PlayOnAwake;
	}

	void AudioSource_SetPlayOnAwake(uint64_t entityID, bool* playOnAwake)
	{
		GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.PlayOnAwake = *playOnAwake;
	}

	void AudioSource_GetLooping(uint64_t entityID, bool* outLooping)
	{
		*outLooping = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.Looping;
	}

	void AudioSource_SetLooping(uint64_t entityID, bool* looping)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.Looping = *looping;
		if (component.Source)
			component.Source->SetLooping(*looping);
	}

	void AudioSource_GetSpatialization(uint64_t entityID, bool* outSpatialization)
	{
		*outSpatialization = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.Spatialization;
	}

	void AudioSource_SetSpatialization(uint64_t entityID, bool* spatialization)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.Spatialization = *spatialization;
		if (component.Source)
			component.Source->SetSpatialization(*spatialization);
	}

	void AudioSource_GetAttenuationModel(uint64_t entityID, int* outAttenuationModel)
	{
		*outAttenuationModel = (int) GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.AttenuationModel;
	}

	void AudioSource_SetAttenuationModel(uint64_t entityID, int* attenuationModel)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.AttenuationModel = (AttenuationModelType)(*attenuationModel);
		if (component.Source)
			component.Source->SetAttenuationModel(component.Config.AttenuationModel);
	}

	void AudioSource_GetRollOff(uint64_t entityID, float* outRollOff)
	{
		*outRollOff = (int)GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.RollOff;
	}

	void AudioSource_SetRollOff(uint64_t entityID, float* rollOff)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.RollOff = *rollOff;
		if (component.Source)
			component.Source->SetRollOff(*rollOff);
	}

	void AudioSource_GetMinGain(uint64_t entityID, float* outMinGain)
	{
		*outMinGain = (int)GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MinGain;
	}

	void AudioSource_SetMinGain(uint64_t entityID, float* minGain)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MinGain = *minGain;
		if (component.Source)
			component.Source->SetMinGain(*minGain);
	}

	void AudioSource_GetMaxGain(uint64_t entityID, float* outMaxGain)
	{
		*outMaxGain = (int)GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MaxGain;
	}

	void AudioSource_SetMaxGain(uint64_t entityID, float* maxGain)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MaxGain = *maxGain;
		if (component.Source)
			component.Source->SetMaxGain(*maxGain);
	}

	void AudioSource_GetMinDistance(uint64_t entityID, float* outMinDistance)
	{
		*outMinDistance = (int)GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MinDistance;
	}

	void AudioSource_SetMinDistance(uint64_t entityID, float* minDistance)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MinDistance = *minDistance;
		if (component.Source)
			component.Source->SetMinDistance(*minDistance);
	}

	void AudioSource_GetMaxDistance(uint64_t entityID, float* outMaxDistance)
	{
		*outMaxDistance = (int)GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MaxDistance;
	}

	void AudioSource_SetMaxDistance(uint64_t entityID, float* maxDistance)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MaxDistance = *maxDistance;
		if (component.Source)
			component.Source->SetMaxDistance(*maxDistance);
	}

	void AudioSource_GetConeInnerAngle(uint64_t entityID, float* outConeInnerAngle)
	{
		*outConeInnerAngle = (int)GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.ConeInnerAngle;
	}

	void AudioSource_SetConeInnerAngle(uint64_t entityID, float* coneInnerAngle)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeInnerAngle = *coneInnerAngle;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	void AudioSource_GetConeOuterAngle(uint64_t entityID, float* outConeOuterAngle)
	{
		*outConeOuterAngle = (int)GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.ConeOuterAngle;
	}

	void AudioSource_SetConeOuterAngle(uint64_t entityID, float* coneOuterAngle)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeOuterAngle = *coneOuterAngle;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	void AudioSource_GetConeOuterGain(uint64_t entityID, float* outConeOuterGain)
	{
		*outConeOuterGain = (int)GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.ConeOuterGain;
	}

	void AudioSource_SetConeOuterGain(uint64_t entityID, float* coneOuterGain)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeOuterGain = *coneOuterGain;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	void AudioSource_SetCone(uint64_t entityID, float* coneInnerAngle, float* coneOuterAngle, float* coneOuterGain)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeInnerAngle = *coneInnerAngle;
		component.Config.ConeOuterAngle = *coneOuterAngle;
		component.Config.ConeOuterGain = *coneOuterGain;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	void AudioSource_GetDopplerFactor(uint64_t entityID, float* outDopplerFactor)
	{
		*outDopplerFactor = (int)GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.DopplerFactor;
	}

	void AudioSource_SetDopplerFactor(uint64_t entityID, float* dopplerFactor)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.DopplerFactor = *dopplerFactor;
		if (component.Source)
			component.Source->SetDopplerFactor(*dopplerFactor);
	}

	void AudioSource_IsPlaying(uint64_t entityID, bool* outIsPlaying)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			*outIsPlaying = component.Source->IsPlaying();
		else
			*outIsPlaying = false;
	}

	void AudioSource_Play(uint64_t entityID)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->Play();
	}

	void AudioSource_Pause(uint64_t entityID)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->Pause();
	}

	void AudioSource_UnPause(uint64_t entityID)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->UnPause();
	}

	void AudioSource_Stop(uint64_t entityID)
	{
		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->Stop();
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

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

		///////////////////////////////////////////////////////////////
		// SpriteRenderer /////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::SpriteRendererComponent_GetColor", SpriteRenderer_GetColor);
		mono_add_internal_call("ArcEngine.InternalCalls::SpriteRendererComponent_SetColor", SpriteRenderer_SetColor);
		mono_add_internal_call("ArcEngine.InternalCalls::SpriteRendererComponent_GetTilingFactor", SpriteRenderer_GetTilingFactor);
		mono_add_internal_call("ArcEngine.InternalCalls::SpriteRendererComponent_SetTilingFactor", SpriteRenderer_SetTilingFactor);

		///////////////////////////////////////////////////////////////
		// Rigidbody 2D ///////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetBodyType", Rigidbody2D_GetBodyType);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetBodyType", Rigidbody2D_SetBodyType);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetAutoMass", Rigidbody2D_GetAutoMass);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetAutoMass", Rigidbody2D_SetAutoMass);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetMass", Rigidbody2D_GetMass);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetMass", Rigidbody2D_SetMass);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetLinearDrag", Rigidbody2D_GetLinearDrag);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetLinearDrag", Rigidbody2D_SetLinearDrag);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetAngularDrag", Rigidbody2D_GetAngularDrag);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetAngularDrag", Rigidbody2D_SetAngularDrag);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetAllowSleep", Rigidbody2D_GetAllowSleep);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetAllowSleep", Rigidbody2D_SetAllowSleep);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetAwake", Rigidbody2D_GetAwake);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetAwake", Rigidbody2D_SetAwake);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetContinuous", Rigidbody2D_GetContinuous);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetContinuous", Rigidbody2D_SetContinuous);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetFreezeRotation", Rigidbody2D_GetFreezeRotation);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetFreezeRotation", Rigidbody2D_SetFreezeRotation);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetGravityScale", Rigidbody2D_GetGravityScale);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetGravityScale", Rigidbody2D_SetGravityScale);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_ApplyForceAtCenter", Rigidbody2D_ApplyForceAtCenter);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_ApplyForce", Rigidbody2D_ApplyForce);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_ApplyLinearImpulse", Rigidbody2D_ApplyLinearImpulse);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_ApplyLinearImpulseAtCenter", Rigidbody2D_ApplyLinearImpulseAtCenter);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_ApplyAngularImpulse", Rigidbody2D_ApplyAngularImpulse);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_ApplyTorque", Rigidbody2D_ApplyTorque);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_IsAwake", Rigidbody2D_IsAwake);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_IsSleeping", Rigidbody2D_IsSleeping);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_MovePosition", Rigidbody2D_MovePosition);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_MoveRotation", Rigidbody2D_MoveRotation);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetVelocity", Rigidbody2D_GetVelocity);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetVelocity", Rigidbody2D_SetVelocity);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_GetAngularVelocity", Rigidbody2D_GetAngularVelocity);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_SetAngularVelocity", Rigidbody2D_SetAngularVelocity);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_Sleep", Rigidbody2D_Sleep);
		mono_add_internal_call("ArcEngine.InternalCalls::Rigidbody2DComponent_WakeUp", Rigidbody2D_WakeUp);

		///////////////////////////////////////////////////////////////
		// Audio Source ///////////////////////////////////////////////
		///////////////////////////////////////////////////////////////
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetVolume", AudioSource_GetVolume);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetVolume", AudioSource_SetVolume);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetPitch", AudioSource_GetPitch);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetPitch", AudioSource_SetPitch);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetPlayOnAwake", AudioSource_GetPlayOnAwake);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetPlayOnAwake", AudioSource_SetPlayOnAwake);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetLooping", AudioSource_GetLooping);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetLooping", AudioSource_SetLooping);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetSpatialization", AudioSource_GetSpatialization);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetSpatialization", AudioSource_SetSpatialization);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetAttenuationModel", AudioSource_GetAttenuationModel);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetAttenuationModel", AudioSource_SetAttenuationModel);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetRollOff", AudioSource_GetRollOff);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetRollOff", AudioSource_SetRollOff);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetMinGain", AudioSource_GetMinGain);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetMinGain", AudioSource_SetMinGain);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetMaxGain", AudioSource_GetMaxGain);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetMaxGain", AudioSource_SetMaxGain);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetMinDistance", AudioSource_GetMinDistance);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetMinDistance", AudioSource_SetMinDistance);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetMaxDistance", AudioSource_GetMaxDistance);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetMaxDistance", AudioSource_SetMaxDistance);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetConeInnerAngle", AudioSource_GetConeInnerAngle);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetConeInnerAngle", AudioSource_SetConeInnerAngle);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetConeOuterAngle", AudioSource_GetConeOuterAngle);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetConeOuterAngle", AudioSource_SetConeOuterAngle);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetConeOuterGain", AudioSource_GetConeOuterGain);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetConeOuterGain", AudioSource_SetConeOuterGain);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetCone", AudioSource_SetCone);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_GetDopplerFactor", AudioSource_GetDopplerFactor);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_SetDopplerFactor", AudioSource_SetDopplerFactor);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_IsPlaying", AudioSource_IsPlaying);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_Play", AudioSource_Play);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_Pause", AudioSource_Pause);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_UnPause", AudioSource_UnPause);
		mono_add_internal_call("ArcEngine.InternalCalls::AudioSource_Stop", AudioSource_Stop);
	}
}
