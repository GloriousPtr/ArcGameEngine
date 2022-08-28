#include "arcpch.h"
#include "AudioListener.h"

#include <miniaudio.h>

#include "AudioEngine.h"

namespace ArcEngine
{
	void AudioListener::SetConfig(const AudioListenerConfig& config, const glm::vec3& position, const glm::vec3 forward)
	{
		const uint32_t listenerIndex = 0;
		ma_engine* engine = (ma_engine*)AudioEngine::GetEngine();
		ma_engine_listener_set_position(engine, listenerIndex, position.x, position.y, position.z);
		ma_engine_listener_set_direction(engine, listenerIndex, forward.x, forward.y, forward.z);
		ma_engine_listener_set_cone(engine, listenerIndex, glm::radians(config.ConeInnerAngle), glm::radians(config.ConeOuterAngle), config.ConeOuterGain);

		static bool setupWorldUp = false;
		if (!setupWorldUp)
		{
			ma_engine_listener_set_world_up(engine, listenerIndex, 0, 1, 0);
			setupWorldUp = true;
		}
	}
}
