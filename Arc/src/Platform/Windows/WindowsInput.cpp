#include "arcpch.h"

#ifdef ARC_PLATFORM_WINDOWS

#include "Arc/Core/Input.h"
#include "Platform/GLFW/GlfwInput.h"

#include "Arc/Core/Application.h"

#include <GLFW/glfw3.h>

namespace ArcEngine
{
bool Input::IsKeyPressed(const KeyCode key)
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		return GlfwInput::IsKeyPressed(key);
	}

	bool Input::IsMouseButtonPressed(const MouseCode button)
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		return GlfwInput::IsMouseButtonPressed(button);
	}

	glm::vec2 Input::GetMousePosition()
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		return GlfwInput::GetMousePosition();
	}

	float Input::GetMouseX()
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		return GetMousePosition().y;
	}

	void Input::SetMousePosition(const glm::vec2& position)
	{
		GlfwInput::SetMousePosition(position);
	}
}

#endif
