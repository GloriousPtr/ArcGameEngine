#pragma once

#include <mutex>
#include <chrono>

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
		explicit Application(const std::string& name = "Arc App");
		virtual ~Application();
		
		Application(const Application& other) = delete;
		Application(Application&& other) = delete;

		void OnEvent(Event& e);

		void PushLayer(Layer* layer) const { m_LayerStack->PushLayer(layer); }
		void PushOverlay(Layer* overlay) const { m_LayerStack->PushOverlay(overlay); }
		void PopLayer(Layer* layer) const { m_LayerStack->PopLayer(layer); }
		void PopOverlay(Layer* overlay) const { m_LayerStack->PopOverlay(overlay); }

		[[nodiscard]] Window& GetWindow() const { return *m_Window; }

		void Close();

		[[nodiscard]] ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }

		[[nodiscard]] static size_t GetAllocatedMemorySize();
		[[nodiscard]] static Application& Get() { return *s_Instance; }
		
		void SubmitToMainThread(const std::function<void()>& function);

	private:
		void Run();
		[[nodiscard]] bool OnWindowClose([[maybe_unused]] const WindowCloseEvent& e);
		[[nodiscard]] bool OnWindowResize(const WindowResizeEvent& e);
		void ExecuteMainThreadQueue();
		
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack* m_LayerStack;
		std::chrono::steady_clock::time_point m_LastFrameTime;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// Should be defined in CLIENT
	Application* CreateApplication();
}
