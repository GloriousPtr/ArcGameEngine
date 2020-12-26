#include "arcpch.h"
#include "Arc/Core/Input.h"

#ifdef ARC_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsInput.h"
#endif

namespace ArcEngine
{
	Scope<Input> Input::s_Instance = Input::Create();

	Scope<Input> Input::Create()
	{
		#ifdef ARC_PLATFORM_WINDOWS
			return CreateScope<WindowsInput>();
		#else
			ARC_CORE_ASSERT(false, "Unknown platform!");
			return nullptr;
		#endif
	}
}

