#pragma once

#include "Arc/Core/Base.h"
#include "Arc/Core/KeyCodes.h"
#include "Arc/Core/MouseCodes.h"

namespace ArcEngine
{
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		
		static bool IsMouseButtonPressed(MouseCode button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}
