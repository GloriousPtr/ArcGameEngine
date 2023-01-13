#include "arcpch.h"
#include "Arc/Core/Window.h"

#include "Platform/GLFW/GlfwWindow.h"

namespace ArcEngine
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#if defined(ARC_PLATFORM_WINDOWS) || defined(ARC_PLATFORM_LINUX)
		return CreateScope<GlfwWindow>(props);
#endif
	}
}
