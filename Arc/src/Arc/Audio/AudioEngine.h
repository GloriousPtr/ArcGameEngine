#pragma once

namespace ArcEngine
{
	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void* GetEngine();
	};
}
