#pragma once

#include "Arc/Core/Window.h"

#include <GLFW/glfw3.h>

namespace ArcEngine
{
	class GraphicsContext;

	class WindowsWindow : public Window
	{
	public:
		explicit WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();
		
		void OnUpdate() override;
		
		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		inline void SetEventCallBack(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		bool IsMaximized() override { return glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED); }
		void Minimize() override;
		void Maximize() override;
		void Restore() override;
		void SetTitleBarRect(const glm::vec4& rect) override;

		inline virtual WindowHandle GetNativeWindow() const { return m_Window; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;
		Scope<GraphicsContext> m_Context;

		struct WindowData
		{
			eastl::string Title;
			unsigned int Width;
			unsigned int Height;
			bool VSync;

			glm::vec4 GrabAreaRect;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;

		static uint8_t s_GLFWWindowCount;
	};
}

