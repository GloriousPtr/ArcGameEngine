#pragma once

#ifdef ARC_PLATFORM_WINDOWS
	#ifdef ARC_BUILD_DLL
		#define ARC_API __declspec(dllexport)
	#else
		#define ARC_API __declspec(dllimport)
	#endif
#else
	#error Arc only support Windows!
#endif

#define BIT(x) (1 << x)
