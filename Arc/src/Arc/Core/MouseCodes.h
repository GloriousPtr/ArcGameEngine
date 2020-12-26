#pragma once

namespace ArcEngine
{
	typedef enum class MouseCode : uint16_t
	{
		// From glfw3.h
		Button0                = 0,
		Button1                = 1,
		Button2                = 2,
		Button3                = 3,
		Button4                = 4,
		Button5                = 5,
		Button6                = 6,
		Button7                = 7,

		ButtonLast             = Button7,
		ButtonLeft             = Button0,
		ButtonRight            = Button1,
		ButtonMiddle           = Button2
	} Mouse;

	inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
	{
		os << static_cast<int32_t>(mouseCode);
		return os;
	}
}

#define ARC_MOUSE_BUTTON_0      ::ArcEngine::Mouse::Button0
#define ARC_MOUSE_BUTTON_1      ::ArcEngine::Mouse::Button1
#define ARC_MOUSE_BUTTON_2      ::ArcEngine::Mouse::Button2
#define ARC_MOUSE_BUTTON_3      ::ArcEngine::Mouse::Button3
#define ARC_MOUSE_BUTTON_4      ::ArcEngine::Mouse::Button4
#define ARC_MOUSE_BUTTON_5      ::ArcEngine::Mouse::Button5
#define ARC_MOUSE_BUTTON_6      ::ArcEngine::Mouse::Button6
#define ARC_MOUSE_BUTTON_7      ::ArcEngine::Mouse::Button7
#define ARC_MOUSE_BUTTON_LAST   ::ArcEngine::Mouse::ButtonLast
#define ARC_MOUSE_BUTTON_LEFT   ::ArcEngine::Mouse::ButtonLeft
#define ARC_MOUSE_BUTTON_RIGHT  ::ArcEngine::Mouse::ButtonRight
#define ARC_MOUSE_BUTTON_MIDDLE ::ArcEngine::Mouse::ButtonMiddle
