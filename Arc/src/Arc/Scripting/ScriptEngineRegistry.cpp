#include "arcpch.h"
#include "ScriptEngineRegistry.h"

#include <glm/gtc/type_ptr.hpp>
#include <box2d/b2_body.h>

#include "Arc/Core/Input.h"
#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Components.h"
#include "ScriptEngine.h"

namespace ArcEngine
{
	void ScriptEngineRegistry::Init()
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_INFO("ScriptEngineRegistry::Init");
	}

#if defined(ARC_PLATFORM_WINDOWS)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#endif

	EXPORT void Log_LogMessage(ArcEngine::Log::Level level, const char* formattedMessage, [[maybe_unused]] const char* filepath, [[maybe_unused]] const char* function, [[maybe_unused]] int32_t line)
	{
		ARC_PROFILE_SCOPE();

		switch (level)
		{
			case ArcEngine::Log::Level::Trace:		ARC_APP_TRACE_EXTERNAL(filepath, line, function, formattedMessage);
				break;
			case ArcEngine::Log::Level::Debug:		ARC_APP_DEBUG_EXTERNAL(filepath, line, function, formattedMessage);
				break;
			case ArcEngine::Log::Level::Info:		ARC_APP_INFO_EXTERNAL(filepath, line, function, formattedMessage);
				break;
			case ArcEngine::Log::Level::Warn:		ARC_APP_WARN_EXTERNAL(filepath, line, function, formattedMessage);
				break;
			case ArcEngine::Log::Level::Error:		ARC_APP_ERROR_EXTERNAL(filepath, line, function, formattedMessage);
				break;
			case ArcEngine::Log::Level::Critical:	ARC_APP_CRITICAL_EXTERNAL(filepath, line, function, formattedMessage);
				break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Input //////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	EXPORT bool Input_IsKeyPressed(KeyCode key)
	{
		ARC_PROFILE_SCOPE();

		return Input::IsKeyPressed(key);
	}

	EXPORT bool Input_IsMouseButtonPressed(MouseCode button)
	{
		ARC_PROFILE_SCOPE();

		return Input::IsMouseButtonPressed(button);
	}

	EXPORT void Input_GetMousePosition(glm::vec2* outMousePosition)
	{
		ARC_PROFILE_SCOPE();

		*outMousePosition = Input::GetMousePosition();
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Entity /////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////
	static Entity GetEntity(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(ScriptEngine::GetScene(), "Active scene is null");
		return ScriptEngine::GetScene()->GetEntity(entityID);
	}

	EXPORT bool Entity_HasComponent(uint64_t entityID, void* type)
	{
		ARC_PROFILE_SCOPE();

		return ScriptEngine::HasComponent(GetEntity(entityID), type);
	}

	EXPORT void Entity_AddComponent(uint64_t entityID, void* type)
	{
		ARC_PROFILE_SCOPE();

		ScriptEngine::AddComponent(GetEntity(entityID), type);
	}

	EXPORT GCHandle Entity_GetComponent(uint64_t entityID, void* type)
	{
		ARC_PROFILE_SCOPE();

		return ScriptEngine::GetComponent(GetEntity(entityID), type);
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Transform //////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	EXPORT void TransformComponent_GetTransform(uint64_t entityID, TransformComponent* outTransform)
	{
		ARC_PROFILE_SCOPE();

		*outTransform = GetEntity(entityID).GetComponent<TransformComponent>();
	}

	EXPORT void TransformComponent_SetTransform(uint64_t entityID, const TransformComponent* inTransform)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<TransformComponent>() = *inTransform;
	}

	EXPORT void TransformComponent_GetTranslation(uint64_t entityID, glm::vec3* outTranslation)
	{
		ARC_PROFILE_SCOPE();

		*outTranslation = GetEntity(entityID).GetComponent<TransformComponent>().Translation;
	}

	EXPORT void TransformComponent_SetTranslation(uint64_t entityID, const glm::vec3* inTranslation)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<TransformComponent>().Translation = *inTranslation;
	}

	EXPORT void TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation)
	{
		ARC_PROFILE_SCOPE();

		*outRotation = GetEntity(entityID).GetComponent<TransformComponent>().Rotation;
	}

	EXPORT void TransformComponent_SetRotation(uint64_t entityID, const glm::vec3* inRotation)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<TransformComponent>().Rotation = *inRotation;
	}

	EXPORT void TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale)
	{
		ARC_PROFILE_SCOPE();

		*outScale = GetEntity(entityID).GetComponent<TransformComponent>().Scale;
	}

	EXPORT void TransformComponent_SetScale(uint64_t entityID, const glm::vec3* inScale)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<TransformComponent>().Scale = *inScale;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Tag ////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	EXPORT const char* TagComponent_GetTag(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& tag = GetEntity(entityID).GetComponent<TagComponent>();
		return tag.Tag.c_str();
	}

	EXPORT void TagComponent_SetTag(uint64_t entityID, const char* tag)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<TagComponent>().Tag = tag;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Sprite Renderer ////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	EXPORT void SpriteRendererComponent_GetColor(uint64_t entityID, glm::vec4* outTint)
	{
		ARC_PROFILE_SCOPE();

		*outTint = GetEntity(entityID).GetComponent<SpriteRendererComponent>().Color;
	}

	EXPORT void SpriteRendererComponent_SetColor(uint64_t entityID, const glm::vec4* tint)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<SpriteRendererComponent>().Color = *tint;
	}

	EXPORT void SpriteRendererComponent_GetTiling(uint64_t entityID, glm::vec2* outTiling)
	{
		ARC_PROFILE_SCOPE();

		*outTiling = GetEntity(entityID).GetComponent<SpriteRendererComponent>().Tiling;
	}

	EXPORT void SpriteRendererComponent_SetTiling(uint64_t entityID, const glm::vec2* tiling)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<SpriteRendererComponent>().Tiling = *tiling;
	}

	EXPORT void SpriteRendererComponent_GetOffset(uint64_t entityID, glm::vec2* outOffset)
	{
		ARC_PROFILE_SCOPE();

		*outOffset = GetEntity(entityID).GetComponent<SpriteRendererComponent>().Offset;
	}

	EXPORT void SpriteRendererComponent_SetOffset(uint64_t entityID, const glm::vec2* outOffset)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<SpriteRendererComponent>().Offset = *outOffset;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Rigid body 2D ///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	inline b2Body* GetB2Body(const Rigidbody2DComponent& component)
	{
		[[likely]]
		if (component.RuntimeBody)
			return static_cast<b2Body*>(component.RuntimeBody);

		return nullptr;
	}

	EXPORT void Rigidbody2DComponent_GetBodyType(uint64_t entityID, int32_t* outType)
	{
		ARC_PROFILE_SCOPE();

		*outType = static_cast<int32_t>(GetEntity(entityID).GetComponent<Rigidbody2DComponent>().Type);
	}

	EXPORT void Rigidbody2DComponent_SetBodyType(uint64_t entityID, const int32_t* type)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.Type = static_cast<Rigidbody2DComponent::BodyType>(*type);
		if (auto* body = GetB2Body(component))
			body->SetType(static_cast<b2BodyType>(*type));
	}

	EXPORT void Rigidbody2DComponent_GetAutoMass(uint64_t entityID, bool* outAutoMass)
	{
		ARC_PROFILE_SCOPE();

		*outAutoMass = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().AutoMass;
	}

	EXPORT void Rigidbody2DComponent_SetAutoMass(uint64_t entityID, const bool* autoMass)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.AutoMass = *autoMass;
		if (auto* body = GetB2Body(component))
		{
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

	EXPORT void Rigidbody2DComponent_GetMass(uint64_t entityID, float* outMass)
	{
		ARC_PROFILE_SCOPE();

		*outMass = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().Mass;
	}

	EXPORT void Rigidbody2DComponent_SetMass(uint64_t entityID, const float* mass)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
		{
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

	EXPORT void Rigidbody2DComponent_GetLinearDrag(uint64_t entityID, float* outDrag)
	{
		ARC_PROFILE_SCOPE();

		*outDrag = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().LinearDrag;
	}

	EXPORT void Rigidbody2DComponent_SetLinearDrag(uint64_t entityID, const float* drag)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.LinearDrag = glm::max(*drag, 0.0f);
		if (auto* body = GetB2Body(component))
			body->SetLinearDamping(component.LinearDrag);
	}

	EXPORT void Rigidbody2DComponent_GetAngularDrag(uint64_t entityID, float* outDrag)
	{
		ARC_PROFILE_SCOPE();

		*outDrag = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().AngularDrag;
	}

	EXPORT void Rigidbody2DComponent_SetAngularDrag(uint64_t entityID, const float* drag)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.AngularDrag = glm::max(*drag, 0.0f);
		if (auto* body = GetB2Body(component))
			body->SetAngularDamping(component.AngularDrag);
	}

	EXPORT void Rigidbody2DComponent_GetAllowSleep(uint64_t entityID, bool* outState)
	{
		ARC_PROFILE_SCOPE();

		*outState = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().AllowSleep;
	}

	EXPORT void Rigidbody2DComponent_SetAllowSleep(uint64_t entityID, const bool* state)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.AllowSleep = *state;
		if (auto* body = GetB2Body(component))
			body->SetSleepingAllowed(component.AllowSleep);
	}

	EXPORT void Rigidbody2DComponent_GetAwake(uint64_t entityID, bool* outState)
	{
		ARC_PROFILE_SCOPE();

		*outState = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().Awake;
	}

	EXPORT void Rigidbody2DComponent_SetAwake(uint64_t entityID, const bool* state)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.Awake = *state;
		if (auto* body = GetB2Body(component))
			body->SetAwake(component.Awake);
	}

	EXPORT void Rigidbody2DComponent_GetContinuous(uint64_t entityID, bool* outState)
	{
		ARC_PROFILE_SCOPE();

		*outState = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().Continuous;
	}

	EXPORT void Rigidbody2DComponent_SetContinuous(uint64_t entityID, const bool* state)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.Continuous = *state;
		if (auto* body = GetB2Body(component))
			body->SetBullet(component.Continuous);
	}

	EXPORT void Rigidbody2DComponent_GetFreezeRotation(uint64_t entityID, bool* outState)
	{
		ARC_PROFILE_SCOPE();

		*outState = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().FreezeRotation;
	}

	EXPORT void Rigidbody2DComponent_SetFreezeRotation(uint64_t entityID, const bool* state)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.FreezeRotation = *state;
		if (auto* body = GetB2Body(component))
			body->SetFixedRotation(component.FreezeRotation);
	}

	EXPORT void Rigidbody2DComponent_GetGravityScale(uint64_t entityID, float* outGravityScale)
	{
		ARC_PROFILE_SCOPE();

		*outGravityScale = GetEntity(entityID).GetComponent<Rigidbody2DComponent>().GravityScale;
	}

	EXPORT void Rigidbody2DComponent_SetGravityScale(uint64_t entityID, const bool* gravityScale)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		component.GravityScale = *gravityScale;
		if (auto* body = GetB2Body(component))
			body->SetGravityScale(component.GravityScale);
	}

	EXPORT void Rigidbody2DComponent_ApplyForceAtCenter(uint64_t entityID, const glm::vec2* force)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->ApplyForceToCenter({ force->x, force->y }, true);
	}

	EXPORT void Rigidbody2DComponent_ApplyForce(uint64_t entityID, const glm::vec2* force, const glm::vec2* point)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->ApplyForce({ force->x, force->y }, { point->x, point->y }, true);
	}

	EXPORT void Rigidbody2DComponent_ApplyLinearImpulse(uint64_t entityID, const glm::vec2* impulse, const glm::vec2* point)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->ApplyLinearImpulse({ impulse->x, impulse->y }, { point->x, point->y }, true);
	}

	EXPORT void Rigidbody2DComponent_ApplyLinearImpulseAtCenter(uint64_t entityID, const glm::vec2* impulse)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->ApplyLinearImpulseToCenter({ impulse->x, impulse->y }, true);
	}

	EXPORT void Rigidbody2DComponent_ApplyAngularImpulse(uint64_t entityID, const float* impulse)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->ApplyAngularImpulse(*impulse, true);
	}

	EXPORT void Rigidbody2DComponent_ApplyTorque(uint64_t entityID, const float* torque)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->ApplyTorque(*torque, true);
	}

	EXPORT void Rigidbody2DComponent_IsAwake(uint64_t entityID, bool* outAwake)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		const auto* body = GetB2Body(component);
		*outAwake = body ? body->IsAwake() : false;
	}

	EXPORT void Rigidbody2DComponent_IsSleeping(uint64_t entityID, bool* outSleeping)
	{
		ARC_PROFILE_SCOPE();

		bool awake;
		Rigidbody2DComponent_IsAwake(entityID, &awake);
		*outSleeping = !awake;
	}

	EXPORT void Rigidbody2DComponent_MovePosition(uint64_t entityID, const glm::vec2* position)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->SetTransform({ position->x, position->y }, body->GetAngle());
	}

	EXPORT void Rigidbody2DComponent_MoveRotation(uint64_t entityID, const float* angleRadians)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->SetTransform(body->GetPosition(), *angleRadians);
	}

	EXPORT void Rigidbody2DComponent_GetVelocity(uint64_t entityID, glm::vec2* outVelocity)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (const auto* body = GetB2Body(component))
		{
			const b2Vec2 velocity = body->GetLinearVelocity();
			outVelocity->x = velocity.x;
			outVelocity->y = velocity.y;
		}
		else
		{
			*outVelocity = glm::vec2(0.0f);
		}
	}

	EXPORT void Rigidbody2DComponent_SetVelocity(uint64_t entityID, const glm::vec2* velocity)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->SetLinearVelocity({ velocity->x, velocity->y });
	}

	EXPORT void Rigidbody2DComponent_GetAngularVelocity(uint64_t entityID, float* outVelocity)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		const auto* body = GetB2Body(component);
		*outVelocity = body ? body->GetAngularVelocity() : 0.0f;
	}

	EXPORT void Rigidbody2DComponent_SetAngularVelocity(uint64_t entityID, const float* velocity)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->SetAngularVelocity(*velocity);
	}

	EXPORT void Rigidbody2DComponent_Sleep(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->SetAwake(false);
	}

	EXPORT void Rigidbody2DComponent_WakeUp(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<Rigidbody2DComponent>();
		if (auto* body = GetB2Body(component))
			body->SetAwake(true);
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Audio Source ////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////

	EXPORT void AudioSourceComponent_GetVolume(uint64_t entityID, float* outVolume)
	{
		ARC_PROFILE_SCOPE();

		*outVolume = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.VolumeMultiplier;
	}

	EXPORT void AudioSourceComponent_SetVolume(uint64_t entityID, const float* volume)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.VolumeMultiplier = *volume;
		if (component.Source)
			component.Source->SetVolume(*volume);
	}

	EXPORT void AudioSourceComponent_GetPitch(uint64_t entityID, float* outPitch)
	{
		ARC_PROFILE_SCOPE();

		*outPitch = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.PitchMultiplier;
	}

	EXPORT void AudioSourceComponent_SetPitch(uint64_t entityID, const float* pitch)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.PitchMultiplier = *pitch;
		if (component.Source)
			component.Source->SetVolume(*pitch);
	}

	EXPORT void AudioSourceComponent_GetPlayOnAwake(uint64_t entityID, bool* outPlayOnAwake)
	{
		ARC_PROFILE_SCOPE();

		*outPlayOnAwake = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.PlayOnAwake;
	}

	EXPORT void AudioSourceComponent_SetPlayOnAwake(uint64_t entityID, const bool* playOnAwake)
	{
		ARC_PROFILE_SCOPE();

		GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.PlayOnAwake = *playOnAwake;
	}

	EXPORT void AudioSourceComponent_GetLooping(uint64_t entityID, bool* outLooping)
	{
		ARC_PROFILE_SCOPE();

		*outLooping = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.Looping;
	}

	EXPORT void AudioSourceComponent_SetLooping(uint64_t entityID, const bool* looping)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.Looping = *looping;
		if (component.Source)
			component.Source->SetLooping(*looping);
	}

	EXPORT void AudioSourceComponent_GetSpatialization(uint64_t entityID, bool* outSpatialization)
	{
		ARC_PROFILE_SCOPE();

		*outSpatialization = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.Spatialization;
	}

	EXPORT void AudioSourceComponent_SetSpatialization(uint64_t entityID, const bool* spatialization)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.Spatialization = *spatialization;
		if (component.Source)
			component.Source->SetSpatialization(*spatialization);
	}

	EXPORT void AudioSourceComponent_GetAttenuationModel(uint64_t entityID, int* outAttenuationModel)
	{
		ARC_PROFILE_SCOPE();

		*outAttenuationModel = static_cast<int>(GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.AttenuationModel);
	}

	EXPORT void AudioSourceComponent_SetAttenuationModel(uint64_t entityID, const int* attenuationModel)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.AttenuationModel = static_cast<AttenuationModelType>(*attenuationModel);
		if (component.Source)
			component.Source->SetAttenuationModel(component.Config.AttenuationModel);
	}

	EXPORT void AudioSourceComponent_GetRollOff(uint64_t entityID, float* outRollOff)
	{
		ARC_PROFILE_SCOPE();

		*outRollOff = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.RollOff;
	}

	EXPORT void AudioSourceComponent_SetRollOff(uint64_t entityID, const float* rollOff)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.RollOff = *rollOff;
		if (component.Source)
			component.Source->SetRollOff(*rollOff);
	}

	EXPORT void AudioSourceComponent_GetMinGain(uint64_t entityID, float* outMinGain)
	{
		ARC_PROFILE_SCOPE();

		*outMinGain = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MinGain;
	}

	EXPORT void AudioSourceComponent_SetMinGain(uint64_t entityID, const float* minGain)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MinGain = *minGain;
		if (component.Source)
			component.Source->SetMinGain(*minGain);
	}

	EXPORT void AudioSourceComponent_GetMaxGain(uint64_t entityID, float* outMaxGain)
	{
		ARC_PROFILE_SCOPE();

		*outMaxGain = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MaxGain;
	}

	EXPORT void AudioSourceComponent_SetMaxGain(uint64_t entityID, const float* maxGain)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MaxGain = *maxGain;
		if (component.Source)
			component.Source->SetMaxGain(*maxGain);
	}

	EXPORT void AudioSourceComponent_GetMinDistance(uint64_t entityID, float* outMinDistance)
	{
		ARC_PROFILE_SCOPE();

		*outMinDistance = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MinDistance;
	}

	EXPORT void AudioSourceComponent_SetMinDistance(uint64_t entityID, const float* minDistance)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MinDistance = *minDistance;
		if (component.Source)
			component.Source->SetMinDistance(*minDistance);
	}

	EXPORT void AudioSourceComponent_GetMaxDistance(uint64_t entityID, float* outMaxDistance)
	{
		ARC_PROFILE_SCOPE();

		*outMaxDistance = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.MaxDistance;
	}

	EXPORT void AudioSourceComponent_SetMaxDistance(uint64_t entityID, const float* maxDistance)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.MaxDistance = *maxDistance;
		if (component.Source)
			component.Source->SetMaxDistance(*maxDistance);
	}

	EXPORT void AudioSourceComponent_GetConeInnerAngle(uint64_t entityID, float* outConeInnerAngle)
	{
		ARC_PROFILE_SCOPE();

		*outConeInnerAngle = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.ConeInnerAngle;
	}

	EXPORT void AudioSourceComponent_SetConeInnerAngle(uint64_t entityID, const float* coneInnerAngle)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeInnerAngle = *coneInnerAngle;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	EXPORT void AudioSourceComponent_GetConeOuterAngle(uint64_t entityID, float* outConeOuterAngle)
	{
		ARC_PROFILE_SCOPE();

		*outConeOuterAngle = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.ConeOuterAngle;
	}

	EXPORT void AudioSourceComponent_SetConeOuterAngle(uint64_t entityID, const float* coneOuterAngle)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeOuterAngle = *coneOuterAngle;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	EXPORT void AudioSourceComponent_GetConeOuterGain(uint64_t entityID, float* outConeOuterGain)
	{
		ARC_PROFILE_SCOPE();

		*outConeOuterGain = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.ConeOuterGain;
	}

	EXPORT void AudioSourceComponent_SetConeOuterGain(uint64_t entityID, const float* coneOuterGain)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeOuterGain = *coneOuterGain;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	EXPORT void AudioSourceComponent_SetCone(uint64_t entityID, const float* coneInnerAngle, const float* coneOuterAngle, const float* coneOuterGain)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.ConeInnerAngle = *coneInnerAngle;
		component.Config.ConeOuterAngle = *coneOuterAngle;
		component.Config.ConeOuterGain = *coneOuterGain;
		if (component.Source)
			component.Source->SetCone(component.Config.ConeInnerAngle, component.Config.ConeOuterAngle, component.Config.ConeOuterGain);
	}

	EXPORT void AudioSourceComponent_GetDopplerFactor(uint64_t entityID, float* outDopplerFactor)
	{
		ARC_PROFILE_SCOPE();

		*outDopplerFactor = GetEntity(entityID).GetComponent<AudioSourceComponent>().Config.DopplerFactor;
	}

	EXPORT void AudioSourceComponent_SetDopplerFactor(uint64_t entityID, const float* dopplerFactor)
	{
		ARC_PROFILE_SCOPE();

		auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		component.Config.DopplerFactor = *dopplerFactor;
		if (component.Source)
			component.Source->SetDopplerFactor(*dopplerFactor);
	}

	EXPORT void AudioSourceComponent_IsPlaying(uint64_t entityID, bool* outIsPlaying)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			*outIsPlaying = component.Source->IsPlaying();
		else
			*outIsPlaying = false;
	}

	EXPORT void AudioSourceComponent_Play(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->Play();
	}

	EXPORT void AudioSourceComponent_Pause(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->Pause();
	}

	EXPORT void AudioSourceComponent_UnPause(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->UnPause();
	}

	EXPORT void AudioSourceComponent_Stop(uint64_t entityID)
	{
		ARC_PROFILE_SCOPE();

		const auto& component = GetEntity(entityID).GetComponent<AudioSourceComponent>();
		if (component.Source)
			component.Source->Stop();
	}
}
