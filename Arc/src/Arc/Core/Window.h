#pragma once

#include <glm/glm.hpp>

#include "Arc/Core/Base.h"
#include "Arc/Events/Event.h"

namespace ArcEngine
{
	using WindowHandle = void*;

	struct WindowProps
	{
		eastl::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const eastl::string& title = "Arc Engine", uint32_t width = 1600, uint32_t height = 900)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = eastl::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallBack(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual bool IsMaximized() = 0;
		virtual void Minimize() = 0;
		virtual void Maximize(const glm::vec2& globalMousePosition) = 0;
		virtual void Restore() = 0;

		virtual glm::vec2 GetPosition() const = 0;
		virtual glm::vec2 GetSize() const = 0;
		virtual glm::vec4 GetMonitorWorkArea(const glm::vec2& globalMousePosition) const = 0;

		virtual void SetPosition(const glm::vec2& position) = 0;
		virtual void Resize(const glm::vec2& position, const glm::vec2& size) = 0;
		virtual void SubmitRestorePosition(const glm::vec2& position) = 0;
		virtual void SubmitRestoreSize(const glm::vec2& size) = 0;

		virtual WindowHandle GetNativeWindow() const = 0;
		
		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}
