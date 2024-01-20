#include "arcpch.h"
#include "Arc/Core/Application.h"

#include <EASTL/chrono.h>

#include "Arc/Audio/AudioEngine.h"
#include "Arc/Core/AssetManager.h"
#include "Arc/Renderer/Renderer.h"
#include "Arc/Scripting/ScriptEngine.h"

extern "C"
{
	ARC_API(dllexport) unsigned long NvOptimusEnablement = 1;
	ARC_API(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace ArcEngine
{
	static Application* s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		ARC_PROFILE_SCOPE();

		ARC_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		
		m_Window = Window::Create(WindowProps(m_Specification.Name));
		m_Window->SetEventCallBack(ARC_BIND_EVENT_FN(Application::OnEvent));

		AssetManager::Init();
		Renderer::Init();
		AudioEngine::Init();
		ScriptEngine::Init();

		m_LayerStack = CreateScope<LayerStack>();
		m_ImGuiLayer = CreateScope<ImGuiLayer>();
		PushOverlay(m_ImGuiLayer.get());
	}

	Application::~Application()
	{
		ARC_PROFILE_SCOPE();

		PopOverlay(m_ImGuiLayer.get());
		m_ImGuiLayer.reset();
		m_LayerStack.reset();

		ScriptEngine::Shutdown();
		AudioEngine::Shutdown();
		Renderer::Shutdown();
		AssetManager::Shutdown();

		OPTICK_SHUTDOWN()
	}

	void Application::Close()
	{
		ARC_PROFILE_SCOPE();

		m_Running = false;
	}

	size_t Application::GetAllocatedMemorySize()
	{
		return Allocation::GetSize();
	}

	Application& Application::Get()
	{
		ARC_CORE_ASSERT(s_Instance, "Application instance doesn't exist!");

		return *s_Instance;
	}
	
	void Application::OnEvent(Event& e)
	{
		ARC_PROFILE_SCOPE();
		
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(ARC_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(ARC_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack->rbegin(); it != m_LayerStack->rend(); ++it)
		{
			if (e.Handled)
				break;
			
			(*it)->OnEvent(e);
		}
	}

	void Application::Run()
	{
		auto lastFrameTime = eastl::chrono::high_resolution_clock::now();
		while (m_Running)
		{
			ARC_PROFILE_FRAME("CPUFrame");
			ARC_PROFILE_THREAD("MainThread");

			ExecuteMainThreadQueue();

			if(!m_Minimized)
			{
				RenderCommand::BeginFrame();
				{
					ARC_PROFILE_SCOPE("LayerStack OnUpdate");

					for (Layer* layer : *m_LayerStack)
						layer->OnUpdate(m_Timestep);	
				}

				m_ImGuiLayer->Begin();
				{
					ARC_PROFILE_SCOPE("LayerStack OnImGuiRender");

					for (Layer* layer : *m_LayerStack)
						layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();
				RenderCommand::EndFrame();
			}
			
			m_Window->OnUpdate();

			const auto time = eastl::chrono::high_resolution_clock::now();
			m_Timestep = eastl::chrono::duration<float>(time - lastFrameTime).count();
			lastFrameTime = time;
		}
	}

	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		m_MainThreadQueue.emplace_back(function);
	}

	bool Application::OnWindowClose([[maybe_unused]] const WindowCloseEvent& e)
	{
		ARC_PROFILE_SCOPE();

		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(const WindowResizeEvent& e)
	{
		ARC_PROFILE_SCOPE();
		
		if(e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		
		return false;
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (const auto& func : m_MainThreadQueue)
			func();

		m_MainThreadQueue.clear();
	}
}
