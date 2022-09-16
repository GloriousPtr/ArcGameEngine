#pragma once

#include "Arc/Core/Window.h"
#include "Arc/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace ArcEngine
{
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

		bool IsMaximized() override { return m_Maximized; }
		void Minimize() override;
		void Maximize() override;
		void Restore() override;
		glm::vec2 GetPosition() override;
		void SetPosition(const glm::vec2& position) override;
		glm::vec2 GetSize() override;
		void Resize(const glm::vec2& position, const glm::vec2& size) override;
		void SubmitRestorePosition(const glm::vec2& position) override;
		void SubmitRestoreSize(const glm::vec2& size) override;

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

			glm::vec2 RestorePosition;
			uint32_t RestoreWidth;
			uint32_t RestoreHeight;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
		bool m_Maximized = false;

		static uint8_t s_GLFWWindowCount;
	};
}

