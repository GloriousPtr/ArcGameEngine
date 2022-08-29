#pragma once

#include <glm/glm.hpp>

namespace ArcEngine
{
	struct AudioListenerConfig
	{
		float ConeInnerAngle = 360.0f;
		float ConeOuterAngle = 360.0f;
		float ConeOuterGain = 0.0f;
	};

	class AudioListener
	{
	public:
		AudioListener() = default;
		~AudioListener() = default;

		void SetConfig(const AudioListenerConfig& config);
		void SetPosition(const glm::vec3& position);
		void SetDirection(const glm::vec3& forward);
		void SetVelocity(const glm::vec3& velocity);

	private:
		uint32_t m_ListenerIndex = 0;
	};
}
