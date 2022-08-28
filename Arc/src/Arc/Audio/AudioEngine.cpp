#include "arcpch.h"
#include "AudioEngine.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace ArcEngine
{
	static ma_engine s_Engine;

	void AudioEngine::Init()
	{
		ma_result result;
		ma_engine_config config = ma_engine_config_init();
		config.listenerCount = 1;

		result = ma_engine_init(nullptr, &s_Engine);
		ARC_CORE_ASSERT(result == MA_SUCCESS, "Failed to initialize audio engine!");
	}

	void AudioEngine::Shutdown()
	{
		ma_engine_uninit(&s_Engine);
	}

	void* AudioEngine::GetEngine()
	{
		return &s_Engine;
	}
}
