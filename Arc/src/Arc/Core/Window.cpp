#include "arcpch.h"
#include "Arc/Core/Window.h"

#include "Platform/GLFW/GlfwWindow.h"
#include "Platform/Dx12/Dx12Window.h"

namespace ArcEngine
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#if defined(ARC_PLATFORM_LINUX)
		return CreateScope<GlfwWindow>(props);
#elif defined(ARC_PLATFORM_WINDOWS)
		return CreateScope<Dx12Window>(props);
#endif
	}
}
