#include "arcpch.h"
#include "AudioEngine.h"

#if defined(__clang__) || defined(__llvm__)
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Weverything"
#elif defined(_MSC_VER)
#	pragma warning(push, 0)
#endif

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#if defined(__clang__) || defined(__llvm__)
#	pragma clang diagnostic pop
#elif defined(_MSC_VER)
#	pragma warning(pop)
#endif

namespace ArcEngine
{
	ma_engine* AudioEngine::s_Engine;

	void AudioEngine::Init()
	{
		ARC_PROFILE_SCOPE()

		ma_engine_config config = ma_engine_config_init();
		config.listenerCount = 1;

		s_Engine = new ma_engine();
		ma_result result = ma_engine_init(nullptr, s_Engine);
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
