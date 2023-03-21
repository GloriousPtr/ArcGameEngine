#include "arcpch.h"
#include "Arc/Core/Window.h"

#include "Platform/GLFW/GlfwWindow.h"

namespace ArcEngine
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
		return CreateScope<GlfwWindow>(props);
	}
}
