#pragma once

#include "Core.h"

#include "LayerStack.h"
#include "Window.h"
#include "Arc/Events/ApplicationEvent.h"

#include "Arc/ImGui/ImGuiLayer.h"

namespace ArcEngine
{
	class ARC_API Application
	{
	public:
		Application();
		virtual ~Application() = default;

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	// Should be defined in CLIENT
	Application* CreateApplication();
}
