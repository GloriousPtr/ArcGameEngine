#pragma once

#include "Arc/Core/Window.h"

namespace ArcEngine
{
	class GraphicsContext;

	class Dx12Window : public Window
	{
		explicit Dx12Window(const WindowProps& props);
		~Dx12Window() override;

		void OnUpdate() override;

		[[nodiscard]] unsigned int GetWidth() const override { return m_Data.Width; }
		[[nodiscard]] unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		void SetEventCallBack(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		[[nodiscard]] bool IsVSync() const override;

		[[nodiscard]] bool IsMaximized() override { return false; }
		void Minimize() override;
		void Maximize() override;
		void Restore() override;
		void RegisterOverTitlebar(bool value) override;

		[[nodiscard]] WindowHandle GetNativeWindow() const override { return m_Window; }
	private:
		void Init(const WindowProps& props);
		void Shutdown() const;

	private:
		WindowHandle m_Window = nullptr;
		Scope<GraphicsContext> m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width = 0;
			unsigned int Height = 0;
			bool VSync = true;

			bool OverTitlebar = false;

			EventCallbackFn EventCallback;

			void* HInstance = nullptr;
		};

		WindowData m_Data;

		static uint8_t s_WindowCount;
	};
}
