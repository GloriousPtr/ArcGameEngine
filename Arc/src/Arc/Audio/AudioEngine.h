#pragma once

struct ma_engine;

namespace ArcEngine
{
	using AudioEngineInternal = void*;

	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static AudioEngineInternal GetEngine();
	};
}
