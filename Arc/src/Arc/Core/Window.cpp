#include "arcpch.h"
#include "Arc/Core/Window.h"

#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Linux/LinuxWindow.h"

namespace ArcEngine
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#if defined(ARC_PLATFORM_WINDOWS)
		return CreateScope<WindowsWindow>(props);
#elif defined(ARC_PLATFORM_LINUX)
		return CreateScope<LinuxWindow>(props);
#endif
	}
}
