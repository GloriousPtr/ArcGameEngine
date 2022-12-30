#include "arcpch.h"

#ifdef ARC_PLATFORM_LINUX

#include "Arc/Core/Input.h"

#include "Arc/Core/Application.h"

#include <GLFW/glfw3.h>

namespace ArcEngine
{
	bool Input::IsKeyPressed(const KeyCode key)
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int state = glfwGetKey(window, static_cast<int32_t>(key));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(const MouseCode button)
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePosition()
	{
		ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos;
		double ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { xpos, ypos };
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
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetCursorPos(window, static_cast<double>(position.x), static_cast<double>(position.y));
	}
}

#endif
