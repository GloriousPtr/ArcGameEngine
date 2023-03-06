#pragma once

#if defined(ARC_PLATFORM_WINDOWS) || defined(ARC_PLATFORM_LINUX)

#include "Arc/Core/Window.h"

#include <GLFW/glfw3.h>

namespace ArcEngine
{
	class GraphicsContext;

	class GlfwWindow : public Window
	{
	public:
		explicit GlfwWindow(const WindowProps& props);
		~GlfwWindow() override;
		
		void OnUpdate() override;

		[[nodiscard]] unsigned int GetWidth() const override { return m_Data.Width; }
		[[nodiscard]] unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		void SetEventCallBack(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		[[nodiscard]] bool IsVSync() const override;

		[[nodiscard]] bool IsMaximized() override { return glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED); }
		void Minimize() override;
		void Maximize() override;
		void Restore() override;
		void RegisterOverTitlebar(bool value) override;

		[[nodiscard]] Scope<GraphicsContext>& GetGraphicsContext() override { return m_Context; }
		[[nodiscard]] WindowHandle GetNativeWindow() const override { return m_Window; }

	private:
		void Init(const WindowProps& props);
		void Shutdown() const;

	private:
		GLFWwindow* m_Window = nullptr;
		Scope<GraphicsContext> m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width = 0;
			unsigned int Height = 0;
			bool VSync = true;

			bool OverTitlebar = false;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;

		static uint8_t s_GLFWWindowCount;
	};
}

#endif
