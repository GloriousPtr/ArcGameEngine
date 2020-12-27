#pragma once

#include "Arc/Core/Base.h"
#include "Arc/Core/Log.h"

#include <filesystem>

#ifdef ARC_ENABLE_ASSERTS

	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define ARC_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { ARC##type##ERROR(msg, __VA_ARGS__); ARC_DEBUGBREAK(); } }
	#define ARC_INTERNAL_ASSERT_WITH_MSG(type, check, ...) ARC_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define ARC_INTERNAL_ASSERT_NO_MSG(type, check) ARC_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", ARC_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define ARC_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define ARC_INTERNAL_ASSERT_GET_MACRO(...) ARC_EXPAND_MACRO( ARC_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, ARC_INTERNAL_ASSERT_WITH_MSG, ARC_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define ARC_ASSERT(...) ARC_EXPAND_MACRO( ARC_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define ARC_CORE_ASSERT(...) ARC_EXPAND_MACRO( ARC_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define ARC_ASSERT(...)
	#define ARC_CORE_ASSERT(...)
#endif
