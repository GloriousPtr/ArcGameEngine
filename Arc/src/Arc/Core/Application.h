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
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			ARC_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		eastl::string Name = "Arc Application";
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		explicit Application(const ApplicationSpecification& specification);
		virtual ~Application();
		
		Application(const Application& other) = delete;
		Application(Application&& other) = delete;

		void OnEvent(Event& e);

		void PushLayer(Layer* layer) const { m_LayerStack->PushLayer(layer); }
		void PushOverlay(Layer* overlay) const { m_LayerStack->PushOverlay(overlay); }
		void PopLayer(Layer* layer) const { m_LayerStack->PopLayer(layer); }
		void PopOverlay(Layer* overlay) const { m_LayerStack->PopOverlay(overlay); }

		[[nodiscard]] Window& GetWindow() const { return *m_Window; }
		[[nodiscard]] const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		void Close();

		[[nodiscard]] ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer.get(); }

		[[nodiscard]] static size_t GetAllocatedMemorySize();
		[[nodiscard]] static Application& Get();
		
		void SubmitToMainThread(const std::function<void()>& function);

	private:
		void Run();
		[[nodiscard]] bool OnWindowClose([[maybe_unused]] const WindowCloseEvent& e);
		[[nodiscard]] bool OnWindowResize(const WindowResizeEvent& e);
		void ExecuteMainThreadQueue();

	private:
		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;
		Scope<ImGuiLayer> m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		Scope<LayerStack> m_LayerStack;
		std::chrono::steady_clock::time_point m_LastFrameTime;

		eastl::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

	private:
		friend int ::main(int argc, char** argv);
	};

	// Should be defined in CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
