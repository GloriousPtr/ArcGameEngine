#pragma once

#include "Arc/Core/PlatformDetection.h"

#ifdef ARC_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See https://github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <sstream>

#include <future>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <stack>
#include <queue>

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
