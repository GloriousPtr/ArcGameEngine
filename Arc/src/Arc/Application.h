#pragma once

#include "Core.h"

#include "LayerStack.h"
#include "Window.h"
#include "Arc/Events/ApplicationEvent.h"

namespace ArcEngine
{
	class ARC_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;
	};

	// Should be defined in CLIENT
	Application* CreateApplication();
}
