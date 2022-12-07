#include "arcpch.h"
#include "ParticleSystem.h"
#include <glm/gtx/compatibility.hpp>

#include "Arc/Renderer/Renderer2D.h"

namespace ArcEngine
{
	void ParticleSystem::OnUpdate(Timestep ts, const glm::vec3& spawnPosition)
	{
		if (m_Particles.empty())
			m_Particles.resize(10000);

		const float dt = ts;

		m_SpawnTime += dt;
		if (m_SpawnTime >= m_Properties.SpawnTime)
		{
			m_SpawnTime = 0.0f;
			Emit(spawnPosition, m_Properties.SpawnRate);
		}
		
		for (auto& particle : m_Particles)
		{
			if (!particle.Active)
				continue;

			if (particle.LifeRemaining <= 0.0f)
			{
				particle.Active = false;
				continue;
			}

			particle.LifeRemaining -= ts;

			float t = particle.LifeRemaining / m_Properties.Lifetime;
			glm::vec3 velocity = glm::lerp(m_Properties.VelocityEnd, m_Properties.VelocityStart, t);
			glm::vec3 force = glm::lerp(m_Properties.ForceEnd, m_Properties.ForceStart, t);
			
			force.y += m_Properties.GravityModifier * -9.8f;
			velocity += force * dt;

			particle.Position += velocity * dt;
		}
	}

	void ParticleSystem::OnRender()
	{
		for (const auto& particle : m_Particles)
		{
			if (particle.LifeRemaining <= 0.0f)
				continue;

			float t = particle.LifeRemaining / m_Properties.Lifetime;
			glm::vec4 color = glm::lerp(m_Properties.ColorEnd, m_Properties.ColorStart, t);
			glm::quat rotation = glm::slerp(glm::quat(m_Properties.RotationEnd), glm::quat(m_Properties.RotationStart), t);
			glm::vec3 size = glm::lerp(m_Properties.SizeEnd, m_Properties.SizeStart, t);

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), particle.Position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), size);
			Renderer2D::DrawQuad(transform, m_Properties.Texture, color);
		}
	}

	static float RandomFloat(float min, float max)
	{
		float r = (float)rand() / (float)RAND_MAX;
		return min + r * (max - min);
	}

	void ParticleSystem::Emit(const glm::vec3& position, uint32_t count)
	{
		for (size_t i = 0; i < count; ++i)
		{
			if (++m_PoolIndex >= 10000)
				m_PoolIndex = 0;

			auto& particle = m_Particles[m_PoolIndex];
			
			particle.Position = position;
			particle.Position.x += RandomFloat(m_Properties.PositionStart.x, m_Properties.PositionEnd.x);
			particle.Position.y += RandomFloat(m_Properties.PositionStart.y, m_Properties.PositionEnd.y);
			particle.Position.z += RandomFloat(m_Properties.PositionStart.z, m_Properties.PositionEnd.z);

			particle.LifeRemaining = m_Properties.Lifetime;
			particle.Active = true;
		}
	}
}
