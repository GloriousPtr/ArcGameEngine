#pragma once

#include <EASTL/fixed_vector.h>
#include <glm/glm.hpp>

#include "Arc/Core/Timestep.h"
#include "Arc/Renderer/Texture.h"

namespace ArcEngine
{
	struct Particle
	{
		glm::vec3 Position = glm::vec3(0.0f);
		float LifeRemaining = 0.0f;
		bool Active = false;

		Particle()
			: Position(glm::vec3(0.0f)), LifeRemaining(0.0f), Active(false)
		{
		}

		Particle(const glm::vec3& position, float life, bool active)
			: Position(position), LifeRemaining(life), Active(active)
		{
		}
	};

	struct ParticleProperties
	{
		float Lifetime = 3.0f;
		float GravityModifier = 0.0f;
		float SpawnTime = 0.2f;
		uint32_t SpawnRate = 10;
		glm::vec3 PositionStart = glm::vec3(-0.2f, 0.0f, 0.0f);
		glm::vec3 PositionEnd = glm::vec3(0.2f, 0.0f, 0.0f);
		
		glm::vec3 VelocityStart = glm::vec3(0.0f, 2.0f, 0.0f);
		glm::vec3 VelocityEnd = glm::vec3(0.0f);
		glm::vec3 ForceStart = glm::vec3(0.0f);
		glm::vec3 ForceEnd = glm::vec3(0.0f);

		glm::vec3 RotationStart = glm::vec3(0.0f);
		glm::vec3 RotationEnd = glm::vec3(0.0f);
		glm::vec3 SizeStart = glm::vec3(0.0f);
		glm::vec3 SizeEnd = glm::vec3(1.0f);
		glm::vec4 ColorStart = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);
		glm::vec4 ColorEnd = glm::vec4(0.2f, 0.2f, 0.75f, 0.0f);
		Ref<Texture2D> Texture = nullptr;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem() = default;
		~ParticleSystem() = default;

		void OnUpdate(Timestep ts, const glm::vec3& spawnPosition);
		void OnRender();

		ParticleProperties& GetProperties() { return m_Properties; }

	private:
		void Emit(const glm::vec3& position, uint32_t count = 1);

	private:
		ParticleProperties m_Properties;
		eastl::fixed_vector<Particle, 1000> m_Particles;
		uint32_t m_PoolIndex = 0;
		float m_SpawnTime = 0.0f;
	};
}
