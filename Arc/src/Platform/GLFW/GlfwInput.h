#include "arcpch.h"

#include "Arc/Core/Application.h"

#include <GLFW/glfw3.h>

namespace ArcEngine
{
    class GlfwInput
    {
    public:
        static bool IsKeyPressed(const KeyCode key)
        {
            ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

            auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
            int state = glfwGetKey(window, static_cast<int32_t>(key));
            return state == GLFW_PRESS || state == GLFW_REPEAT;
        }

        static bool IsMouseButtonPressed(const MouseCode button)
        {
            ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

            auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
            int state = glfwGetMouseButton(window, static_cast<int32_t>(button));
            return state == GLFW_PRESS;
        }

        static glm::vec2 GetMousePosition()
        {
            ARC_PROFILE_CATEGORY("Input", Profile::Category::Input);

            auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
            double xpos;
            double ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            return { xpos, ypos };
        }

        static void SetMousePosition(const glm::vec2& position)
        {
            auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
            glfwSetCursorPos(window, static_cast<double>(position.x), static_cast<double>(position.y));
        }
    };
}
