#pragma once

#include <glm/vec2.hpp>

#include "Arc/Core/KeyCodes.h"
#include "Arc/Core/MouseCodes.h"

namespace ArcEngine
{
	class Input
	{
	public:
		[[nodiscard]] static bool IsKeyPressed(KeyCode key);
		
		[[nodiscard]] static bool IsMouseButtonPressed(MouseCode button);
		[[nodiscard]] static glm::vec2 GetMousePosition();
		static void SetMousePosition(const glm::vec2& position);

		[[nodiscard]] static float GetMouseX() { return GetMousePosition().x; }
		[[nodiscard]] static float GetMouseY() { return GetMousePosition().y; }
	};
}
