#include "arcpch.h"
#include "Arc/Core/Window.h"

#ifdef ARC_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif

namespace ArcEngine
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
	#ifdef ARC_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
	#else
		ARC_CORE_ASSERT(false, "Unknown platform!")
		return nullptr;
	#endif
	}

}

