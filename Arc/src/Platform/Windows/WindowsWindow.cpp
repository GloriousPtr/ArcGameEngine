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
		m_Data.RestoreWidth = props.Width;
		m_Data.RestoreHeight = props.Height;
		m_Data.RestorePosition = { 0.0f, 0.0f };
		
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

			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
			m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
			++s_GLFWWindowCount;
		}
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

	void WindowsWindow::Maximize(const glm::vec2& globalMousePosition)
	{
		int monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

		int x = 0;
		int y = 0;
		int width = 1;
		int height = 1;
		uint32_t monitorIndex = -1;
		for (int i = 0; i < monitorCount; ++i)
		{
			glfwGetMonitorWorkarea(monitors[i], &x, &y, &width, &height);
			if (globalMousePosition.x < (float)(x + width) && globalMousePosition.y < (float)(y + height))
			{
				monitorIndex = i;
				break;
			}
		}

		if (m_MaximizedMonitor == monitorIndex || monitorIndex < 0)
			return;

		glfwSetWindowPos(m_Window, x, y);
		glfwSetWindowSize(m_Window, width, height - 1);
		m_MaximizedMonitor = monitorIndex;
	}

	void WindowsWindow::Restore()
	{
		if (m_MaximizedMonitor >= 0)
		{
			glfwSetWindowPos(m_Window, (int32_t)m_Data.RestorePosition.x, (int32_t)m_Data.RestorePosition.y);
			glfwSetWindowSize(m_Window, m_Data.RestoreWidth, m_Data.RestoreHeight);
			m_MaximizedMonitor = -1;
		}
	}

	glm::vec2 WindowsWindow::GetPosition() const
	{
		int x;
		int y;
		glfwGetWindowPos(m_Window, &x, &y);
		return glm::vec2((float)x, (float)y);
	}

	glm::vec2 WindowsWindow::GetSize() const
	{
		int width;
		int height;
		glfwGetWindowSize(m_Window, &width, &height);
		return { (float)width, (float)height };
	}

	void WindowsWindow::SetPosition(const glm::vec2& position)
	{
		glfwSetWindowPos(m_Window, (int)position.x, (int)position.y);
	}

	glm::vec4 WindowsWindow::GetMonitorWorkArea(const glm::vec2& globalMousePosition) const
	{
		int monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

		int x = 0;
		int y = 0;
		int width = 1;
		int height = 1;
		for (int i = 0; i < monitorCount; ++i)
		{
			glfwGetMonitorWorkarea(monitors[i], &x, &y, &width, &height);
			if (globalMousePosition.x < (float)(x + width) && globalMousePosition.y < (float)(y + height))
				return { x, y, width, height };
		}

		ARC_CORE_ASSERT(false);
		return { 0, 0, 1, 1 };
	}

	void WindowsWindow::Resize(const glm::vec2& position, const glm::vec2& size)
	{
		glfwSetWindowPos(m_Window, (const int32_t)position.x, (const int32_t)position.y);
		glfwSetWindowSize(m_Window, (const int32_t)size.x, (const int32_t)size.y);
	}

	void WindowsWindow::SubmitRestorePosition(const glm::vec2& position)
	{
		m_Data.RestorePosition = position;
	}

	void WindowsWindow::SubmitRestoreSize(const glm::vec2& size)
	{
		m_Data.RestoreWidth = (const int32_t)size.x;
		m_Data.RestoreHeight = (const int32_t)size.y;
	}
}
