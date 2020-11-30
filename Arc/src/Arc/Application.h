#pragma once

#include "Core.h"
#include "Window.h"

namespace ArcEngine
{
	class ARC_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// Should be defined in CLIENT
	Application* CreateApplication();
}
