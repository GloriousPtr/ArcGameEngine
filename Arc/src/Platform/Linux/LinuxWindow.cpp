#include "arcpch.h"

#ifdef ARC_PLATFORM_LINUX

#include "Platform/Linux/LinuxWindow.h"

#include "Arc/Events/ApplicationEvent.h"
#include "Arc/Events/MouseEvent.h"
#include "Arc/Events/KeyEvent.h"

#include "Arc/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLContext.h"

#include "Arc/Core/Input.h"

namespace ArcEngine
{
	uint8_t LinuxWindow::s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		ARC_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}
	
	LinuxWindow::LinuxWindow(const WindowProps& props)
	{
		ARC_PROFILE_SCOPE()
		
		Init(props);
	}
	
	LinuxWindow::~LinuxWindow()
	{
		ARC_PROFILE_SCOPE()
		
		Shutdown();
	}

	void LinuxWindow::Init(const WindowProps& props)
	{
		ARC_PROFILE_SCOPE()
		
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		
		ARC_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);
		
		if(s_GLFWWindowCount == 0)
		{
			ARC_PROFILE_SCOPE("glfwInit")
			
			int success = glfwInit();
			ARC_CORE_ASSERT(success, "Could not initialize GLFW!")
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		{
			ARC_PROFILE_SCOPE("glfwCreateWindow")
			#ifdef ARC_DEBUG
				if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
					glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
			#endif

			m_Window = glfwCreateWindow(static_cast<int>(props.Width), static_cast<int>(props.Height), m_Data.Title.c_str(), nullptr, nullptr);
			++s_GLFWWindowCount;
		}

		Maximize();
		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();
		
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input)

			auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->Width = width;
			data->Height = height;

			WindowResizeEvent event(width, height);
			data->EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input)

			const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			WindowCloseEvent event;
			data->EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input)

			const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 0);
					data->EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					data->EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 1);
					data->EventCallback(event);
					break;
				}
				default:
					break;
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input)

			const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			KeyTypedEvent event(static_cast<KeyCode>(keycode));
			data->EventCallback(event);
		});
		
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input)

			const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(static_cast<uint16_t>(button));
					data->EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(static_cast<uint16_t>(button));
					data->EventCallback(event);
					break;
				}
				default:
					break;
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input)

			const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
			data->EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input)

			const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
			data->EventCallback(event);
		});
	}

	void LinuxWindow::Shutdown() const
	{
		ARC_PROFILE_SCOPE()
		
		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;
		
		if(s_GLFWWindowCount == 0)
			glfwTerminate();
	}

	void LinuxWindow::OnUpdate()
	{
		ARC_PROFILE_SCOPE()
		
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input)
			glfwPollEvents();
		}

		{
			ARC_PROFILE_CATEGORY("Wait", Profile::Category::Wait)
			m_Context->SwapBuffers();
		}
	}

	void LinuxWindow::SetVSync(bool enabled)
	{
		ARC_PROFILE_SCOPE()
		
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool LinuxWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
	
	void LinuxWindow::Minimize()
	{
		glfwIconifyWindow(m_Window);
	}

	void LinuxWindow::Maximize()
	{
		glfwMaximizeWindow(m_Window);
	}

	void LinuxWindow::Restore()
	{
		glfwRestoreWindow(m_Window);
	}

	void LinuxWindow::RegisterOverTitlebar(bool value)
	{
	}
}

#endif
