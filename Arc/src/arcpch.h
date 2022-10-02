#pragma once

#include "Arc/Core/PlatformDetection.h"

#ifdef ARC_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <sstream>

#include <EASTL/memory.h>
#include <EASTL/utility.h>
#include <EASTL/algorithm.h>
#include <EASTL/functional.h>
#include <EASTL/string.h>
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/hash_map.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Arc/Core/Base.h"

#include "Arc/Core/Log.h"

#include "Arc/Debug/Profiler.h"

#ifdef ARC_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

namespace ArcEngine::Allocation
{
	size_t GetSize();
}
