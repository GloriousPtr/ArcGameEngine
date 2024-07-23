#pragma once

//#include <Window.h>

namespace ArcEngine
{
	class Stopwatch
	{
	public:
		static void Init()
		{
			QueryPerformanceFrequency(&Frequency);
		}

		Stopwatch()
			: StartCounter(__rdtsc())
		{
		}

		inline void Reset()
		{
			StartCounter = __rdtsc();
			QueryPerformanceCounter(&StartTimer);
		}

		// In seconds
		inline float Stop()
		{
			EndCounter = __rdtsc();
			QueryPerformanceCounter(&EndTimer);
			return (float) (EndTimer.QuadPart - StartTimer.QuadPart) / Frequency.QuadPart;
		}

		uint64_t StartCounter;
		uint64_t EndCounter;
		LARGE_INTEGER StartTimer;
		LARGE_INTEGER EndTimer;

		inline static LARGE_INTEGER Frequency;
	};
}
