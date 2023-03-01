#include "arcpch.h"

#if defined(ARC_PLATFORM_WINDOWS)

#include "Arc/Core/Application.h"
#include "Arc/Core/Input.h"

namespace ArcEngine
{
	bool Input::IsKeyPressed(const KeyCode key)
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		auto hwnd = static_cast<HWND>(Application::Get().GetWindow().GetNativeWindow());
		return (GetAsyncKeyState(static_cast<int>(key)) & 0x8000) && (GetForegroundWindow() == hwnd);
	}

	bool Input::IsMouseButtonPressed(const MouseCode button)
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		auto hwnd = static_cast<HWND>(Application::Get().GetWindow().GetNativeWindow());
		return (GetAsyncKeyState(static_cast<int>(button)) & 0x8000) && (GetForegroundWindow() == hwnd);
	}

	glm::vec2 Input::GetMousePosition()
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		POINT cursorPos;
		GetCursorPos(&cursorPos);
		POINT clientPos = cursorPos;
		auto hwnd = static_cast<HWND>(Application::Get().GetWindow().GetNativeWindow());
		ScreenToClient(hwnd, &clientPos);
		return { clientPos.x, clientPos.y };
	}

	void Input::SetMousePosition(const glm::vec2& position)
	{
		auto hwnd = static_cast<HWND>(Application::Get().GetWindow().GetNativeWindow());
		RECT rect;
		GetWindowRect(hwnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		int screen_x = rect.left + static_cast<int>(position.x) * width / 65535;
		int screen_y = rect.top + static_cast<int>(position.y) * height / 65535;
		SetCursorPos(screen_x, screen_y);
	}
}

#endif
