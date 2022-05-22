#include "arcpch.h"
#include "Arc/Core/Application.h"

#include "Arc/Renderer/Renderer.h"
#include "Arc/Scripting/ScriptEngine.h"

#include <GLFW/glfw3.h>
#include <optick.config.h>

namespace ArcEngine
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const eastl::string& name)
	{
		ARC_PROFILE_SCOPE();
		
		ARC_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		
		m_Window = Window::Create(WindowProps(name));
		m_Window->SetEventCallBack(ARC_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();
		ScriptEngine::Init("Resources/Scripts/Arc-ScriptCore.dll");

		m_LayerStack = new LayerStack();
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		ARC_PROFILE_SCOPE();
		
		delete m_LayerStack;

		ScriptEngine::Shutdown();
		Renderer::Shutdown();

		OPTICK_SHUTDOWN();
	}

	void Application::PushLayer(Layer* layer)
	{
		ARC_PROFILE_SCOPE();

		m_LayerStack->PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		ARC_PROFILE_SCOPE();
		
		m_LayerStack->PushOverlay(layer);
		layer->OnAttach();
	}
	
	void Application::Close()
	{
		ARC_PROFILE_SCOPE();

		m_Running = false;
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
		while (m_Running)
		{
			ARC_PROFILE_FRAME("MainThread");
			
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if(!m_Minimized)
			{
				{
					ARC_PROFILE_SCOPE("LayerStack OnUpdate");

					for (Layer* layer : *m_LayerStack)
						layer->OnUpdate(timestep);	
				}

				m_ImGuiLayer->Begin();
				{
					ARC_PROFILE_SCOPE("LayerStack OnImGuiRender");

					for (Layer* layer : *m_LayerStack)
						layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();
			}
			
			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		ARC_PROFILE_SCOPE();

		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
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
}
