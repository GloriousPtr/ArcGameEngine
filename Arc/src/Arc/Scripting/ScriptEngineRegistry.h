#pragma once

#include <Arc/Core/KeyCodes.h>
#include <Arc/Core/MouseCodes.h>

namespace ArcEngine
{
	using GCHandle = void*;

	struct TransformComponent;

	class ScriptEngineRegistry
	{
	public:
		static void Init();
	};

	ARC_EXPORT void Log_LogMessage(ArcEngine::Log::Level level, const char* formattedMessage, const char* filepath, const char* function, int32_t line);

	ARC_EXPORT bool Input_IsKeyPressed(KeyCode key);
	ARC_EXPORT bool Input_IsMouseButtonPressed(MouseCode button);
	ARC_EXPORT void Input_GetMousePosition(glm::vec2* outMousePosition);

	ARC_EXPORT bool Entity_HasComponent(uint64_t entityID, void* type);
	ARC_EXPORT void Entity_AddComponent(uint64_t entityID, void* type);
	ARC_EXPORT GCHandle Entity_GetComponent(uint64_t entityID, void* type);

	ARC_EXPORT void TransformComponent_GetTransform(uint64_t entityID, TransformComponent* outTransform);
	ARC_EXPORT void TransformComponent_SetTransform(uint64_t entityID, const TransformComponent* inTransform);
	ARC_EXPORT void TransformComponent_GetTranslation(uint64_t entityID, glm::vec3* outTranslation);
	ARC_EXPORT void TransformComponent_SetTranslation(uint64_t entityID, const glm::vec3* inTranslation);
	ARC_EXPORT void TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation);
	ARC_EXPORT void TransformComponent_SetRotation(uint64_t entityID, const glm::vec3* inRotation);
	ARC_EXPORT void TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale);
	ARC_EXPORT void TransformComponent_SetScale(uint64_t entityID, const glm::vec3* inScale);

	ARC_EXPORT const char* TagComponent_GetTag(uint64_t entityID);
	ARC_EXPORT void TagComponent_SetTag(uint64_t entityID, const char* tag);

	ARC_EXPORT void SpriteRendererComponent_GetColor(uint64_t entityID, glm::vec4* outTint);
	ARC_EXPORT void SpriteRendererComponent_SetColor(uint64_t entityID, const glm::vec4* tint);
	ARC_EXPORT void SpriteRendererComponent_GetTiling(uint64_t entityID, glm::vec2* outTiling);
	ARC_EXPORT void SpriteRendererComponent_SetTiling(uint64_t entityID, const glm::vec2* tiling);
	ARC_EXPORT void SpriteRendererComponent_GetOffset(uint64_t entityID, glm::vec2* outOffset);
	ARC_EXPORT void SpriteRendererComponent_SetOffset(uint64_t entityID, const glm::vec2* outOffset);

	ARC_EXPORT void Rigidbody2DComponent_GetBodyType(uint64_t entityID, int32_t* outType);
	ARC_EXPORT void Rigidbody2DComponent_SetBodyType(uint64_t entityID, const int32_t* type);
	ARC_EXPORT void Rigidbody2DComponent_GetAutoMass(uint64_t entityID, bool* outAutoMass);
	ARC_EXPORT void Rigidbody2DComponent_SetAutoMass(uint64_t entityID, const bool* autoMass);
	ARC_EXPORT void Rigidbody2DComponent_GetMass(uint64_t entityID, float* outMass);
	ARC_EXPORT void Rigidbody2DComponent_SetMass(uint64_t entityID, const float* mass);
	ARC_EXPORT void Rigidbody2DComponent_GetLinearDrag(uint64_t entityID, float* outDrag);
	ARC_EXPORT void Rigidbody2DComponent_SetLinearDrag(uint64_t entityID, const float* drag);
	ARC_EXPORT void Rigidbody2DComponent_GetAngularDrag(uint64_t entityID, float* outDrag);
	ARC_EXPORT void Rigidbody2DComponent_SetAngularDrag(uint64_t entityID, const float* drag);
	ARC_EXPORT void Rigidbody2DComponent_GetAllowSleep(uint64_t entityID, bool* outState);
	ARC_EXPORT void Rigidbody2DComponent_SetAllowSleep(uint64_t entityID, const bool* state);
	ARC_EXPORT void Rigidbody2DComponent_GetAwake(uint64_t entityID, bool* outState);
	ARC_EXPORT void Rigidbody2DComponent_SetAwake(uint64_t entityID, const bool* state);
	ARC_EXPORT void Rigidbody2DComponent_GetContinuous(uint64_t entityID, bool* outState);
	ARC_EXPORT void Rigidbody2DComponent_SetContinuous(uint64_t entityID, const bool* state);
	ARC_EXPORT void Rigidbody2DComponent_GetFreezeRotation(uint64_t entityID, bool* outState);
	ARC_EXPORT void Rigidbody2DComponent_SetFreezeRotation(uint64_t entityID, const bool* state);
	ARC_EXPORT void Rigidbody2DComponent_GetGravityScale(uint64_t entityID, float* outGravityScale);
	ARC_EXPORT void Rigidbody2DComponent_SetGravityScale(uint64_t entityID, const float* gravityScale);
	ARC_EXPORT void Rigidbody2DComponent_ApplyForceAtCenter(uint64_t entityID, const glm::vec2* force);
	ARC_EXPORT void Rigidbody2DComponent_ApplyForce(uint64_t entityID, const glm::vec2* force, const glm::vec2* point);
	ARC_EXPORT void Rigidbody2DComponent_ApplyLinearImpulse(uint64_t entityID, const glm::vec2* impulse, const glm::vec2* point);
	ARC_EXPORT void Rigidbody2DComponent_ApplyLinearImpulseAtCenter(uint64_t entityID, const glm::vec2* impulse);
	ARC_EXPORT void Rigidbody2DComponent_ApplyAngularImpulse(uint64_t entityID, const float* impulse);
	ARC_EXPORT void Rigidbody2DComponent_ApplyTorque(uint64_t entityID, const float* torque);
	ARC_EXPORT void Rigidbody2DComponent_IsAwake(uint64_t entityID, bool* outAwake);
	ARC_EXPORT void Rigidbody2DComponent_IsSleeping(uint64_t entityID, bool* outSleeping);
	ARC_EXPORT void Rigidbody2DComponent_MovePosition(uint64_t entityID, const glm::vec2* position);
	ARC_EXPORT void Rigidbody2DComponent_MoveRotation(uint64_t entityID, const float* angleRadians);
	ARC_EXPORT void Rigidbody2DComponent_GetVelocity(uint64_t entityID, glm::vec2* outVelocity);
	ARC_EXPORT void Rigidbody2DComponent_SetVelocity(uint64_t entityID, const glm::vec2* velocity);
	ARC_EXPORT void Rigidbody2DComponent_GetAngularVelocity(uint64_t entityID, float* outVelocity);
	ARC_EXPORT void Rigidbody2DComponent_SetAngularVelocity(uint64_t entityID, const float* velocity);
	ARC_EXPORT void Rigidbody2DComponent_Sleep(uint64_t entityID);
	ARC_EXPORT void Rigidbody2DComponent_WakeUp(uint64_t entityID);

	ARC_EXPORT void AudioSourceComponent_GetVolume(uint64_t entityID, float* outVolume);
	ARC_EXPORT void AudioSourceComponent_SetVolume(uint64_t entityID, const float* volume);
	ARC_EXPORT void AudioSourceComponent_GetPitch(uint64_t entityID, float* outPitch);
	ARC_EXPORT void AudioSourceComponent_SetPitch(uint64_t entityID, const float* pitch);
	ARC_EXPORT void AudioSourceComponent_GetPlayOnAwake(uint64_t entityID, bool* outPlayOnAwake);
	ARC_EXPORT void AudioSourceComponent_SetPlayOnAwake(uint64_t entityID, const bool* playOnAwake);
	ARC_EXPORT void AudioSourceComponent_GetLooping(uint64_t entityID, bool* outLooping);
	ARC_EXPORT void AudioSourceComponent_SetLooping(uint64_t entityID, const bool* looping);
	ARC_EXPORT void AudioSourceComponent_GetSpatialization(uint64_t entityID, bool* outSpatialization);
	ARC_EXPORT void AudioSourceComponent_SetSpatialization(uint64_t entityID, const bool* spatialization);
	ARC_EXPORT void AudioSourceComponent_GetAttenuationModel(uint64_t entityID, int* outAttenuationModel);
	ARC_EXPORT void AudioSourceComponent_SetAttenuationModel(uint64_t entityID, const int* attenuationModel);
	ARC_EXPORT void AudioSourceComponent_GetRollOff(uint64_t entityID, float* outRollOff);
	ARC_EXPORT void AudioSourceComponent_SetRollOff(uint64_t entityID, const float* rollOff);
	ARC_EXPORT void AudioSourceComponent_GetMinGain(uint64_t entityID, float* outMinGain);
	ARC_EXPORT void AudioSourceComponent_SetMinGain(uint64_t entityID, const float* minGain);
	ARC_EXPORT void AudioSourceComponent_GetMaxGain(uint64_t entityID, float* outMaxGain);
	ARC_EXPORT void AudioSourceComponent_SetMaxGain(uint64_t entityID, const float* maxGain);
	ARC_EXPORT void AudioSourceComponent_GetMinDistance(uint64_t entityID, float* outMinDistance);
	ARC_EXPORT void AudioSourceComponent_SetMinDistance(uint64_t entityID, const float* minDistance);
	ARC_EXPORT void AudioSourceComponent_GetMaxDistance(uint64_t entityID, float* outMaxDistance);
	ARC_EXPORT void AudioSourceComponent_SetMaxDistance(uint64_t entityID, const float* maxDistance);
	ARC_EXPORT void AudioSourceComponent_GetConeInnerAngle(uint64_t entityID, float* outConeInnerAngle);
	ARC_EXPORT void AudioSourceComponent_SetConeInnerAngle(uint64_t entityID, const float* coneInnerAngle);
	ARC_EXPORT void AudioSourceComponent_GetConeOuterAngle(uint64_t entityID, float* outConeOuterAngle);
	ARC_EXPORT void AudioSourceComponent_SetConeOuterAngle(uint64_t entityID, const float* coneOuterAngle);
	ARC_EXPORT void AudioSourceComponent_GetConeOuterGain(uint64_t entityID, float* outConeOuterGain);
	ARC_EXPORT void AudioSourceComponent_SetConeOuterGain(uint64_t entityID, const float* coneOuterGain);
	ARC_EXPORT void AudioSourceComponent_SetCone(uint64_t entityID, const float* coneInnerAngle, const float* coneOuterAngle, const float* coneOuterGain);
	ARC_EXPORT void AudioSourceComponent_GetDopplerFactor(uint64_t entityID, float* outDopplerFactor);
	ARC_EXPORT void AudioSourceComponent_SetDopplerFactor(uint64_t entityID, const float* dopplerFactor);
	ARC_EXPORT void AudioSourceComponent_IsPlaying(uint64_t entityID, bool* outIsPlaying);
	ARC_EXPORT void AudioSourceComponent_Play(uint64_t entityID);
	ARC_EXPORT void AudioSourceComponent_Pause(uint64_t entityID);
	ARC_EXPORT void AudioSourceComponent_UnPause(uint64_t entityID);
	ARC_EXPORT void AudioSourceComponent_Stop(uint64_t entityID);
}
