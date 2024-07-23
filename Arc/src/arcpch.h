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

#include <EASTL/span.h>
#include <EASTL/memory.h>
#include <EASTL/utility.h>
#include <EASTL/algorithm.h>
#include <EASTL/functional.h>
#include <EASTL/string.h>
#include <EASTL/array.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/vector.h>
#include <EASTL/vector_map.h>
#include <EASTL/hash_map.h>
#include <EASTL/map.h>
#include <EASTL/unordered_map.h>
#include <EASTL/set.h>
#include <EASTL/unordered_set.h>
#include <EASTL/stack.h>
#include <EASTL/queue.h>

#include <future>
#include <string>
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
	uint64_t GetSize();
	void* New(size_t size);
	void Free(void* ptr, size_t size);
}
