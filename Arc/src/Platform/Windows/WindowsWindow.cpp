#include "arcpch.h"
#include "Platform/Windows/WindowsWindow.h"

#include "Arc/Events/ApplicationEvent.h"
#include "Arc/Events/MouseEvent.h"
#include "Arc/Events/KeyEvent.h"

#include "Arc/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLContext.h"

#include "Arc/Core/Input.h"

namespace ArcEngine
{
	uint8_t WindowsWindow::s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		ARC_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}
	
	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		ARC_PROFILE_SCOPE();
		
		Init(props);
	}
	
	WindowsWindow::~WindowsWindow()
	{
		ARC_PROFILE_SCOPE();
		
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		ARC_PROFILE_SCOPE();
		
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		
		ARC_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);
		
		if(s_GLFWWindowCount == 0)
		{
			ARC_PROFILE_SCOPE("glfwInit");
			
			int success = glfwInit();
			ARC_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		{
			ARC_PROFILE_SCOPE("glfwCreateWindow");
			#if defined(ARC_DEBUG)
				if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
					glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
			#endif

			glfwWindowHint(GLFW_TITLEBAR, GLFW_FALSE);
			m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
			++s_GLFWWindowCount;
		}

		Maximize();
		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();
		
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event((uint16_t)key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event((uint16_t)key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event((uint16_t)key, 1);
					data.EventCallback(event);
					break;
				}
				default:
					break;
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event((uint16_t)keycode);
			data.EventCallback(event);
		});
		
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event((uint16_t)button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event((uint16_t)button);
					data.EventCallback(event);
					break;
				}
				default:
					break;
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});

		glfwSetTitlebarHitTestCallback(m_Window, [](GLFWwindow* window, int xPos, int yPos, int* hit)
		{
			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			if ((data.GrabAreaRect.x < (float)xPos && (float)xPos < data.GrabAreaRect.z) &&
				(data.GrabAreaRect.y < (float)yPos && (float)yPos < data.GrabAreaRect.w))
			{
				*hit = 1;
			}
		});
	}

	void WindowsWindow::Shutdown()
	{
		ARC_PROFILE_SCOPE();
		
		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;
		
		if(s_GLFWWindowCount == 0)
			glfwTerminate();
	}

	void WindowsWindow::OnUpdate()
	{
		ARC_PROFILE_SCOPE();
		
		{
			ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);
			glfwPollEvents();
		}

		{
			ARC_PROFILE_CATEGORY("Wait", Profile::Category::Wait);
			m_Context->SwapBuffers();
		}
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		ARC_PROFILE_SCOPE();
		
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
	
	void WindowsWindow::Minimize()
	{
		glfwIconifyWindow(m_Window);
	}

	void WindowsWindow::Maximize()
	{
		glfwMaximizeWindow(m_Window);
	}

	void WindowsWindow::Restore()
	{
		glfwRestoreWindow(m_Window);
	}

	void WindowsWindow::SetTitleBarRect(const glm::vec4& rect)
	{
		m_Data.GrabAreaRect = rect;
	}
}
