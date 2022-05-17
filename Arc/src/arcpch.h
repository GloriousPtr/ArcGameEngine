#pragma once

#include "Arc/Core/PlatformDetection.h"

#ifdef ARC_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif


#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "Arc/Core/Base.h"

#include "Arc/Core/Log.h"

#include "Arc/Debug/Profiler.h"

#ifdef ARC_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

