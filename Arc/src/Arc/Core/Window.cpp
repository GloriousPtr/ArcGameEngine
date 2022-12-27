#include "arcpch.h"
#include "Arc/Core/Window.h"

#include "Platform/Windows/WindowsWindow.h"

namespace ArcEngine
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
		return CreateScope<WindowsWindow>(props);
	}

}

