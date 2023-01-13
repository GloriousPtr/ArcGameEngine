#include "arcpch.h"
#include "AudioEngine.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace ArcEngine
{
	ma_engine* AudioEngine::s_Engine;

	void AudioEngine::Init()
	{
		ARC_PROFILE_SCOPE()

		ma_engine_config config = ma_engine_config_init();
		config.listenerCount = 1;

		s_Engine = new ma_engine();
		[[maybe_unused]] const ma_result result = ma_engine_init(&config, s_Engine);
		ARC_CORE_ASSERT(result == MA_SUCCESS, "Failed to initialize audio engine!")
	}

	void AudioEngine::Shutdown()
	{
		ARC_PROFILE_SCOPE()

		ma_engine_uninit(s_Engine);
		delete s_Engine;
	}

	AudioEngineInternal AudioEngine::GetEngine()
	{
		return s_Engine;
	}
}
