#pragma once

namespace ArcEngine
{
	struct AudioListenerConfig
	{
		float ConeInnerAngle = glm::radians(360.0f);
		float ConeOuterAngle = glm::radians(360.0f);
		float ConeOuterGain = 0.0f;
	};

	class AudioListener
	{
	public:
		AudioListener() = default;

		void SetConfig(const AudioListenerConfig& config) const;
		void SetPosition(const glm::vec3& position) const;
		void SetDirection(const glm::vec3& forward) const;
		void SetVelocity(const glm::vec3& velocity) const;

	private:
		uint32_t m_ListenerIndex = 0;
	};
}
