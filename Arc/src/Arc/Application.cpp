#include "arcpch.h"
#include "Application.h"

#include "Arc/Events/ApplicationEvent.h"
#include "Arc/Log.h"

namespace ArcEngine
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		ARC_TRACE(e);
		
		while (true);
	}
}
