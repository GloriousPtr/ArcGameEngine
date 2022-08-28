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

		void SetConfig(const AudioListenerConfig& config, const glm::vec3& position, const glm::vec3 forward);
	};
}
