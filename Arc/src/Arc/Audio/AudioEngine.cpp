#include "arcpch.h"
#include "AudioEngine.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace ArcEngine
{
	static Scope<ma_engine> s_Engine;

	void AudioEngine::Init()
	{
		ARC_PROFILE_SCOPE();

		ma_engine_config config = ma_engine_config_init();
		config.listenerCount = 1;

		s_Engine = CreateScope<ma_engine>();
		[[maybe_unused]] const ma_result result = ma_engine_init(&config, s_Engine.get());
		ARC_CORE_ASSERT(result == MA_SUCCESS, "Failed to initialize audio engine!");
	}

	void AudioEngine::Shutdown()
	{
		ARC_PROFILE_SCOPE();

		ma_engine_uninit(s_Engine.get());
		s_Engine.reset();
	}

	AudioEngineInternal AudioEngine::GetEngine()
	{
		return s_Engine.get();
	}
}
