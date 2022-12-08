#pragma once

#include <EASTL/fixed_vector.h>
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

#include "Arc/Core/Timestep.h"
#include "Arc/Math/Math.h"
#include "Arc/Renderer/Texture.h"

namespace ArcEngine
{
	struct Particle
	{
		glm::vec3 Position;
		glm::vec3 Rotation;
		glm::vec3 Size;
		glm::vec4 Color;
		float LifeRemaining;
		bool Active;

		Particle()
			: Position(glm::vec3(0.0f)), Color(glm::vec4(1.0f)), LifeRemaining(0.0f), Active(false)
		{
		}

		Particle(const glm::vec3& position, const glm::vec4 color, float life, bool active)
			: Position(position), Color(color), LifeRemaining(life), Active(active)
		{
		}
	};

	template<typename T>
	struct OverLifetimeModule
	{
		T Start;
		T End;
		bool Enabled = false;

		OverLifetimeModule()
		{
		}

		OverLifetimeModule(const T& start, const T& end)
			: Start(start), End(end)
		{
		}

		T Evaluate(float factor)
		{
			return glm::lerp(End, Start, factor);
		}
	};

	template<typename T>
	struct BySpeedModule
	{
		T Start;
		T End;
		float MinSpeed = 0.0f;
		float MaxSpeed = 1.0f;
		bool Enabled = false;

		BySpeedModule()
		{
		}

		BySpeedModule(const T& start, const T& end)
			: Start(start), End(end)
		{
		}

		T Evaluate(float speed)
		{
			float factor = Math::InverseLerpClamped(MinSpeed, MaxSpeed, speed);
			return glm::lerp(End, Start, factor);
		}
	};

	struct ParticleProperties
	{
		float Duration = 3.0f;
		bool Looping = true;
		float StartDelay = 0.0f;
		float StartLifetime = 3.0f;
		glm::vec3 StartVelocity = glm::vec3(0.0f, 2.0f, 0.0f);
		glm::vec4 StartColor = glm::vec4(1.0f);
		glm::vec3 StartSize = glm::vec3(1.0f);
		glm::vec3 StartRotation = glm::vec3(0.0f);
		float GravityModifier = 0.0f;
		float SimulationSpeed = 1.0f;
		bool PlayOnAwake = true;
		uint32_t MaxParticles = 1000;

		uint16_t RateOverTime = 10;
		uint16_t RateOverDistance = 0;
		uint16_t BurstCount = 0;
		float BurstTime = 1.0f;

		glm::vec3 PositionStart = glm::vec3(-0.2f, 0.0f, 0.0f);
		glm::vec3 PositionEnd = glm::vec3(0.2f, 0.0f, 0.0f);
		
		OverLifetimeModule<glm::vec3> VelocityOverLifetime;
		OverLifetimeModule<glm::vec3> ForceOverLifetime;
		OverLifetimeModule<glm::vec4> ColorOverLifetime = { { 0.8f, 0.2f, 0.2f, 0.0f }, { 0.2f, 0.2f, 0.75f, 1.0f } };
		BySpeedModule<glm::vec4> ColorBySpeed = { { 0.8f, 0.2f, 0.2f, 0.0f }, { 0.2f, 0.2f, 0.75f, 1.0f } };
		OverLifetimeModule<glm::vec3> SizeOverLifetime = { glm::vec3(0.2f), glm::vec3(1.0f) };
		BySpeedModule<glm::vec3> SizeBySpeed = { glm::vec3(0.2f), glm::vec3(1.0f) };
		OverLifetimeModule<glm::vec3> RotationOverLifetime;
		BySpeedModule<glm::vec3> RotationBySpeed;

		Ref<Texture2D> Texture = nullptr;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem() = default;
		~ParticleSystem() = default;

		void OnUpdate(Timestep ts, const glm::vec3& position);
		void OnRender();

		ParticleProperties& GetProperties() { return m_Properties; }

	private:
		void Emit(const glm::vec3& position, uint16_t count = 1);

	private:
		ParticleProperties m_Properties;
		eastl::fixed_vector<Particle, 1000> m_Particles;
		uint32_t m_PoolIndex = 0;

		float m_BurstTime = 0.0f;
		float m_SpawnTime = 0.0f;
		glm::vec3 m_LastSpawnedPosition = glm::vec3(0.0f);

		uint32_t m_ActiveParticleCount = 0;
	};
}
