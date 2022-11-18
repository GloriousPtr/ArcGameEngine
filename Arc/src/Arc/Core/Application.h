#pragma once

#include <mutex>

#include "Arc/Core/Base.h"

#include "Arc/Core/Window.h"
#include "Arc/Core/LayerStack.h"
#include "Arc/Events/ApplicationEvent.h"

#include "Arc/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace ArcEngine
{
	class Application
	{
	public:
		explicit Application(const eastl::string& name = "Arc App");
		virtual ~Application();
		
		Application(const Application& other) = delete;
		Application(Application&& other) = delete;

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		
		inline Window& GetWindow() const { return *m_Window; }

		void Close();

		ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }

		static size_t GetAllocatedMemorySize();
		inline static Application& Get() { return *s_Instance; }
		
		void SubmitToMainThread(const std::function<void()>& function);

	private:
		void Run();
		bool OnWindowClose([[maybe_unused]] const WindowCloseEvent& e);
		bool OnWindowResize(const WindowResizeEvent& e);
		void ExecuteMainThreadQueue();
		
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack* m_LayerStack;
		float m_LastFrameTime = 0.0f;

		eastl::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// Should be defined in CLIENT
	Application* CreateApplication();
}
