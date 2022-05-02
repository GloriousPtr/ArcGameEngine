#pragma once

#include "Arc/Core/Base.h"

#include "Arc/Core/Window.h"
#include "Arc/Core/LayerStack.h"
#include "Arc/Events/ApplicationEvent.h"

#include "Arc/Core/Timestep.h"

#include "Arc/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace ArcEngine
{
	class Application
	{
	public:
		Application(const std::string& name = "Arc App");
		virtual ~Application();
		
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		
		inline Window& GetWindow() { return *m_Window; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		inline static Application& Get() { return *s_Instance; }
		
	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack* m_LayerStack;
		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// Should be defined in CLIENT
	Application* CreateApplication();
}
