#pragma once

#include "Arc/Core/PlatformDetection.h"

#ifdef ARC_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See https://github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <fstream>
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
#include <ranges>

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <magic_enum.hpp>

#include <stb_image.h>

#include <yaml-cpp/yaml.h>

#include "Arc/Core/Base.h"
#include "Arc/Core/Log.h"
#include "Arc/Debug/Profiler.h"

#ifdef ARC_PLATFORM_WINDOWS
	#include <Windows.h>
	#include <atlbase.h>
	#include <comutil.h>
	#include <wrl.h>
#endif

namespace ArcEngine::Allocation
{
	size_t GetSize();
}
