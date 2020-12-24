#pragma once

#include <memory>

#ifdef ARC_PLATFORM_WINDOWS
	#if ARC_DYNAMIC_LINK
		#ifdef ARC_BUILD_DLL
			#define ARC_API __declspec(dllexport)
		#else
			#define ARC_API __declspec(dllimport)
		#endif
	#else
		#define ARC_API
	#endif
#else
	#error Arc only support Windows!
#endif

#ifdef ARC_DEBUG
	#define ARC_ENABLE_ASSERTS
#endif

#ifdef ARC_ENABLE_ASSERTS
	#define ARC_ASSERT(x, ...) { if(!(x)) { ARC_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define ARC_CORE_ASSERT(x, ...) { if(!(x)) { ARC_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define ARC_ASSERT(x, ...) 
	#define ARC_CORE_ASSERT(x, ...) 
#endif

#define BIT(x) (1 << x)

#define ARC_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace ArcEngine
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;
	
}
