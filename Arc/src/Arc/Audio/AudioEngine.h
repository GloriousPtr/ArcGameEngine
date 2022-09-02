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

	private:
		static ma_engine* s_Engine;
	};
}
