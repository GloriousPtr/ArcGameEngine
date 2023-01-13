#include "arcpch.h"
#include "ParticleSystem.h"

#include <glm/gtx/norm.hpp>

#include "Arc/Renderer/Renderer2D.h"

namespace ArcEngine
{
	ParticleSystem::ParticleSystem()
		: m_Particles(10000)
	{
		if (m_Properties.PlayOnAwake)
			Play();
	}

	void ParticleSystem::Play()
	{
		m_SystemTime = 0.0f;
		m_Playing = true;
	}

	void ParticleSystem::Stop(bool force)
	{
		if (force)
		{
			for (auto& particle : m_Particles)
				particle.LifeRemaining = 0.0f;
		}

		m_SystemTime = m_Properties.StartDelay + m_Properties.Duration;
		m_Playing = false;
	}

	void ParticleSystem::OnUpdate(Timestep ts, const glm::vec3& position)
	{
		ARC_PROFILE_SCOPE()

		const float simTs = ts * m_Properties.SimulationSpeed;

		if (m_Playing && !m_Properties.Looping)
			m_SystemTime += simTs;
		const float delay = m_Properties.StartDelay;
		if (m_Playing && (m_Properties.Looping || (m_SystemTime <= delay + m_Properties.Duration && m_SystemTime > delay)))
		{
			// Emit particles in unit time
			m_SpawnTime += simTs;
			if (m_SpawnTime >= 1.0f / static_cast<float>(m_Properties.RateOverTime))
			{
				m_SpawnTime = 0.0f;
				Emit(position, 1);
			}

			// Emit particles over unit distance
			if (glm::distance2(m_LastSpawnedPosition, position) > 1.0f)
			{
				m_LastSpawnedPosition = position;
				Emit(position, m_Properties.RateOverDistance);
			}

			// Emit bursts of particles over time
			m_BurstTime += simTs;
			if (m_BurstTime >= m_Properties.BurstTime)
			{
				m_BurstTime = 0.0f;
				Emit(position, m_Properties.BurstCount);
			}
		}
		
		// Simulate
		m_ActiveParticleCount = 0;
		for (auto& particle : m_Particles)
		{
			if (particle.LifeRemaining <= 0.0f)
				continue;

			particle.LifeRemaining -= simTs;

			const float t = glm::clamp(particle.LifeRemaining / m_Properties.StartLifetime, 0.0f, 1.0f);
			
			glm::vec3 velocity = m_Properties.StartVelocity;
			if (m_Properties.VelocityOverLifetime.Enabled)
				velocity *= m_Properties.VelocityOverLifetime.Evaluate(t);

			glm::vec3 force(0.0f);
			if (m_Properties.ForceOverLifetime.Enabled)
				force = m_Properties.ForceOverLifetime.Evaluate(t);

			force.y += m_Properties.GravityModifier * -9.8f;
			velocity += force * simTs;

			const float velocityMagnitude = glm::length(velocity);
			
			// Color
			particle.Color = m_Properties.StartColor;
			if (m_Properties.ColorOverLifetime.Enabled)
				particle.Color *= m_Properties.ColorOverLifetime.Evaluate(t);
			if (m_Properties.ColorBySpeed.Enabled)
				particle.Color *= m_Properties.ColorBySpeed.Evaluate(velocityMagnitude);

			// Size
			particle.Size = m_Properties.StartSize;
			if (m_Properties.SizeOverLifetime.Enabled)
				particle.Size *= m_Properties.SizeOverLifetime.Evaluate(t);
			if (m_Properties.SizeBySpeed.Enabled)
				particle.Size *= m_Properties.SizeBySpeed.Evaluate(velocityMagnitude);

			// Rotation
			particle.Rotation = m_Properties.StartRotation;
			if (m_Properties.RotationOverLifetime.Enabled)
				particle.Rotation += m_Properties.RotationOverLifetime.Evaluate(t);
			if (m_Properties.RotationBySpeed.Enabled)
				particle.Rotation += m_Properties.RotationBySpeed.Evaluate(velocityMagnitude);

			particle.Position += velocity * simTs;
			++m_ActiveParticleCount;
		}
	}

	void ParticleSystem::OnRender() const
	{
		ARC_PROFILE_SCOPE()

		for (const auto& particle : m_Particles)
		{
			if (particle.LifeRemaining <= 0.0f)
				continue;

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), particle.Position) * glm::mat4(glm::quat(particle.Rotation)) * glm::scale(glm::mat4(1.0f), particle.Size);
			Renderer2D::DrawQuad(transform, m_Properties.Texture, particle.Color);
		}
	}

	static float RandomFloat(float min, float max)
	{
		const float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
		return min + r * (max - min);
	}

	void ParticleSystem::Emit(const glm::vec3& position, uint32_t count)
	{
		ARC_PROFILE_SCOPE()

		if (m_ActiveParticleCount >= m_Properties.MaxParticles)
			return;

		for (uint32_t i = 0; i < count; ++i)
		{
			if (++m_PoolIndex >= 10000)
				m_PoolIndex = 0;

			auto& particle = m_Particles[m_PoolIndex];
			
			particle.Position = position;
			particle.Position.x += RandomFloat(m_Properties.PositionStart.x, m_Properties.PositionEnd.x);
			particle.Position.y += RandomFloat(m_Properties.PositionStart.y, m_Properties.PositionEnd.y);
			particle.Position.z += RandomFloat(m_Properties.PositionStart.z, m_Properties.PositionEnd.z);

			particle.LifeRemaining = m_Properties.StartLifetime;
		}
	}
}
