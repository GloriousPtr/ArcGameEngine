#pragma once

#include <glm/glm.hpp>

#include "Arc/Core/KeyCodes.h"
#include "Arc/Core/MouseCodes.h"

namespace ArcEngine
{
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		
		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
		static void SetMousePosition(const glm::vec2& position);
	};
}
