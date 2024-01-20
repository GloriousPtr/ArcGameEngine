#pragma once

#include <EASTL/chrono.h>

namespace ArcEngine
{
	class Stopwatch
	{
	public:
		Stopwatch()
			: m_Start(eastl::chrono::high_resolution_clock::now())
		{
		}

		inline void Reset()
		{
			m_Start = eastl::chrono::high_resolution_clock::now();
		}

		// In seconds
		inline float Stop()
		{
			return eastl::chrono::duration<float>(eastl::chrono::high_resolution_clock::now() - m_Start).count();
		}

	private:
		eastl::chrono::time_point<eastl::chrono::high_resolution_clock> m_Start {};
	};
}
