#pragma once

#include <EASTL/memory.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>

#include "Arc/Core/PlatformDetection.h"

#ifdef ARC_DEBUG
	#if defined(ARC_PLATFORM_WINDOWS)
		#define ARC_DEBUGBREAK() __debugbreak()
	#elif defined(ARC_PLATFORM_LINUX)
		#include <signal.h>
		#define ARC_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define ARC_ENABLE_ASSERTS
#else
	#define ARC_DEBUGBREAK()
#endif

#define ARC_EXPAND_MACRO(x) x
#define ARC_STRINGIFY_MACRO(x) #x

#define ARC_CONCAT(X,Y) ARC_CONCAT_IMPL(X,Y)
#define ARC_CONCAT_IMPL(X,Y) X##Y

#define CONSTEVAL_LINE int(ARC_CONCAT(__LINE__,U))

#define BIT(x) (1 << x)

#define ARC_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	#define ARC_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
	#define ARC_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
	#define ARC_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	#define ARC_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	#define ARC_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	#define ARC_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
	#define ARC_FUNC_SIG __func__
#else
	#define ARC_FUNC_SIG "ARC_FUNC_SIG unknown!"
#endif

namespace ArcEngine
{
	template<typename T>
	using Scope = eastl::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return eastl::make_unique<T>(eastl::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = eastl::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return eastl::make_shared<T>(eastl::forward<Args>(args)...);
	}
}

#include "Arc/Core/Log.h"
#include "Arc/Core/Assert.h"
