#pragma once

namespace ArcEngine
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space               = 32,
		Apostrophe          = 39, /* ' */
		Comma               = 44, /* , */
		Minus               = 45, /* - */
		Period              = 46, /* . */
		Slash               = 47, /* / */

		D0                  = 48, /* 0 */
		D1                  = 49, /* 1 */
		D2                  = 50, /* 2 */
		D3                  = 51, /* 3 */
		D4                  = 52, /* 4 */
		D5                  = 53, /* 5 */
		D6                  = 54, /* 6 */
		D7                  = 55, /* 7 */
		D8                  = 56, /* 8 */
		D9                  = 57, /* 9 */

		Semicolon           = 59, /* ; */
		Equal               = 61, /* = */

		A                   = 65,
		B                   = 66,
		C                   = 67,
		D                   = 68,
		E                   = 69,
		F                   = 70,
		G                   = 71,
		H                   = 72,
		I                   = 73,
		J                   = 74,
		K                   = 75,
		L                   = 76,
		M                   = 77,
		N                   = 78,
		O                   = 79,
		P                   = 80,
		Q                   = 81,
		R                   = 82,
		S                   = 83,
		T                   = 84,
		U                   = 85,
		V                   = 86,
		W                   = 87,
		X                   = 88,
		Y                   = 89,
		Z                   = 90,

		LeftBracket         = 91,  /* [ */
		Backslash           = 92,  /* \ */
		RightBracket        = 93,  /* ] */
		GraveAccent         = 96,  /* ` */

		World1              = 161, /* non-US #1 */
		World2              = 162, /* non-US #2 */

		/* Function keys */
		Escape              = 256,
		Enter               = 257,
		Tab                 = 258,
		Backspace           = 259,
		Insert              = 260,
		Delete              = 261,
		Right               = 262,
		Left                = 263,
		Down                = 264,
		Up                  = 265,
		PageUp              = 266,
		PageDown            = 267,
		Home                = 268,
		End                 = 269,
		CapsLock            = 280,
		ScrollLock          = 281,
		NumLock             = 282,
		PrintScreen         = 283,
		Pause               = 284,
		F1                  = 290,
		F2                  = 291,
		F3                  = 292,
		F4                  = 293,
		F5                  = 294,
		F6                  = 295,
		F7                  = 296,
		F8                  = 297,
		F9                  = 298,
		F10                 = 299,
		F11                 = 300,
		F12                 = 301,
		F13                 = 302,
		F14                 = 303,
		F15                 = 304,
		F16                 = 305,
		F17                 = 306,
		F18                 = 307,
		F19                 = 308,
		F20                 = 309,
		F21                 = 310,
		F22                 = 311,
		F23                 = 312,
		F24                 = 313,
		F25                 = 314,

		/* Keypad */
		KP0                 = 320,
		KP1                 = 321,
		KP2                 = 322,
		KP3                 = 323,
		KP4                 = 324,
		KP5                 = 325,
		KP6                 = 326,
		KP7                 = 327,
		KP8                 = 328,
		KP9                 = 329,
		KPDecimal           = 330,
		KPDivide            = 331,
		KPMultiply          = 332,
		KPSubtract          = 333,
		KPAdd               = 334,
		KPEnter             = 335,
		KPEqual             = 336,

		LeftShift           = 340,
		LeftControl         = 341,
		LeftAlt             = 342,
		LeftSuper           = 343,
		RightShift          = 344,
		RightControl        = 345,
		RightAlt            = 346,
		RightSuper          = 347,
		Menu                = 348
	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}
}

#define ARC_KEY_SPACE           ::ArcEngine::Key::Space
#define ARC_KEY_APOSTROPHE      ::ArcEngine::Key::Apostrophe    /* ' */
#define ARC_KEY_COMMA           ::ArcEngine::Key::Comma         /* , */
#define ARC_KEY_MINUS           ::ArcEngine::Key::Minus         /* - */
#define ARC_KEY_PERIOD          ::ArcEngine::Key::Period        /* . */
#define ARC_KEY_SLASH           ::ArcEngine::Key::Slash         /* / */
#define ARC_KEY_0               ::ArcEngine::Key::D0
#define ARC_KEY_1               ::ArcEngine::Key::D1
#define ARC_KEY_2               ::ArcEngine::Key::D2
#define ARC_KEY_3               ::ArcEngine::Key::D3
#define ARC_KEY_4               ::ArcEngine::Key::D4
#define ARC_KEY_5               ::ArcEngine::Key::D5
#define ARC_KEY_6               ::ArcEngine::Key::D6
#define ARC_KEY_7               ::ArcEngine::Key::D7
#define ARC_KEY_8               ::ArcEngine::Key::D8
#define ARC_KEY_9               ::ArcEngine::Key::D9
#define ARC_KEY_SEMICOLON       ::ArcEngine::Key::Semicolon     /* ; */
#define ARC_KEY_EQUAL           ::ArcEngine::Key::Equal         /* = */
#define ARC_KEY_A               ::ArcEngine::Key::A
#define ARC_KEY_B               ::ArcEngine::Key::B
#define ARC_KEY_C               ::ArcEngine::Key::C
#define ARC_KEY_D               ::ArcEngine::Key::D
#define ARC_KEY_E               ::ArcEngine::Key::E
#define ARC_KEY_F               ::ArcEngine::Key::F
#define ARC_KEY_G               ::ArcEngine::Key::G
#define ARC_KEY_H               ::ArcEngine::Key::H
#define ARC_KEY_I               ::ArcEngine::Key::I
#define ARC_KEY_J               ::ArcEngine::Key::J
#define ARC_KEY_K               ::ArcEngine::Key::K
#define ARC_KEY_L               ::ArcEngine::Key::L
#define ARC_KEY_M               ::ArcEngine::Key::M
#define ARC_KEY_N               ::ArcEngine::Key::N
#define ARC_KEY_O               ::ArcEngine::Key::O
#define ARC_KEY_P               ::ArcEngine::Key::P
#define ARC_KEY_Q               ::ArcEngine::Key::Q
#define ARC_KEY_R               ::ArcEngine::Key::R
#define ARC_KEY_S               ::ArcEngine::Key::S
#define ARC_KEY_T               ::ArcEngine::Key::T
#define ARC_KEY_U               ::ArcEngine::Key::U
#define ARC_KEY_V               ::ArcEngine::Key::V
#define ARC_KEY_W               ::ArcEngine::Key::W
#define ARC_KEY_X               ::ArcEngine::Key::X
#define ARC_KEY_Y               ::ArcEngine::Key::Y
#define ARC_KEY_Z               ::ArcEngine::Key::Z
#define ARC_KEY_LEFT_BRACKET    ::ArcEngine::Key::LeftBracket   /* [ */
#define ARC_KEY_BACKSLASH       ::ArcEngine::Key::Backslash     /* \ */
#define ARC_KEY_RIGHT_BRACKET   ::ArcEngine::Key::RightBracket  /* ] */
#define ARC_KEY_GRAVE_ACCENT    ::ArcEngine::Key::GraveAccent   /* ` */
#define ARC_KEY_WORLD_1         ::ArcEngine::Key::World1        /* non-US #1 */
#define ARC_KEY_WORLD_2         ::ArcEngine::Key::World2        /* non-US #2 */

/* Function keys */
#define ARC_KEY_ESCAPE          ::ArcEngine::Key::Escape
#define ARC_KEY_ENTER           ::ArcEngine::Key::Enter
#define ARC_KEY_TAB             ::ArcEngine::Key::Tab
#define ARC_KEY_BACKSPACE       ::ArcEngine::Key::Backspace
#define ARC_KEY_INSERT          ::ArcEngine::Key::Insert
#define ARC_KEY_DELETE          ::ArcEngine::Key::Delete
#define ARC_KEY_RIGHT           ::ArcEngine::Key::Right
#define ARC_KEY_LEFT            ::ArcEngine::Key::Left
#define ARC_KEY_DOWN            ::ArcEngine::Key::Down
#define ARC_KEY_UP              ::ArcEngine::Key::Up
#define ARC_KEY_PAGE_UP         ::ArcEngine::Key::PageUp
#define ARC_KEY_PAGE_DOWN       ::ArcEngine::Key::PageDown
#define ARC_KEY_HOME            ::ArcEngine::Key::Home
#define ARC_KEY_END             ::ArcEngine::Key::End
#define ARC_KEY_CAPS_LOCK       ::ArcEngine::Key::CapsLock
#define ARC_KEY_SCROLL_LOCK     ::ArcEngine::Key::ScrollLock
#define ARC_KEY_NUM_LOCK        ::ArcEngine::Key::NumLock
#define ARC_KEY_PRINT_SCREEN    ::ArcEngine::Key::PrintScreen
#define ARC_KEY_PAUSE           ::ArcEngine::Key::Pause
#define ARC_KEY_F1              ::ArcEngine::Key::F1
#define ARC_KEY_F2              ::ArcEngine::Key::F2
#define ARC_KEY_F3              ::ArcEngine::Key::F3
#define ARC_KEY_F4              ::ArcEngine::Key::F4
#define ARC_KEY_F5              ::ArcEngine::Key::F5
#define ARC_KEY_F6              ::ArcEngine::Key::F6
#define ARC_KEY_F7              ::ArcEngine::Key::F7
#define ARC_KEY_F8              ::ArcEngine::Key::F8
#define ARC_KEY_F9              ::ArcEngine::Key::F9
#define ARC_KEY_F10             ::ArcEngine::Key::F10
#define ARC_KEY_F11             ::ArcEngine::Key::F11
#define ARC_KEY_F12             ::ArcEngine::Key::F12
#define ARC_KEY_F13             ::ArcEngine::Key::F13
#define ARC_KEY_F14             ::ArcEngine::Key::F14
#define ARC_KEY_F15             ::ArcEngine::Key::F15
#define ARC_KEY_F16             ::ArcEngine::Key::F16
#define ARC_KEY_F17             ::ArcEngine::Key::F17
#define ARC_KEY_F18             ::ArcEngine::Key::F18
#define ARC_KEY_F19             ::ArcEngine::Key::F19
#define ARC_KEY_F20             ::ArcEngine::Key::F20
#define ARC_KEY_F21             ::ArcEngine::Key::F21
#define ARC_KEY_F22             ::ArcEngine::Key::F22
#define ARC_KEY_F23             ::ArcEngine::Key::F23
#define ARC_KEY_F24             ::ArcEngine::Key::F24
#define ARC_KEY_F25             ::ArcEngine::Key::F25

/* KeypaARC*/
#define ARC_KEY_KP_0            ::ArcEngine::Key::KP0
#define ARC_KEY_KP_1            ::ArcEngine::Key::KP1
#define ARC_KEY_KP_2            ::ArcEngine::Key::KP2
#define ARC_KEY_KP_3            ::ArcEngine::Key::KP3
#define ARC_KEY_KP_4            ::ArcEngine::Key::KP4
#define ARC_KEY_KP_5            ::ArcEngine::Key::KP5
#define ARC_KEY_KP_6            ::ArcEngine::Key::KP6
#define ARC_KEY_KP_7            ::ArcEngine::Key::KP7
#define ARC_KEY_KP_8            ::ArcEngine::Key::KP8
#define ARC_KEY_KP_9            ::ArcEngine::Key::KP9
#define ARC_KEY_KP_DECIMAL      ::ArcEngine::Key::KPDecimal
#define ARC_KEY_KP_DIVIDE       ::ArcEngine::Key::KPDivide
#define ARC_KEY_KP_MULTIPLY     ::ArcEngine::Key::KPMultiply
#define ARC_KEY_KP_SUBTRACT     ::ArcEngine::Key::KPSubtract
#define ARC_KEY_KP_ADD          ::ArcEngine::Key::KPAdd
#define ARC_KEY_KP_ENTER        ::ArcEngine::Key::KPEnter
#define ARC_KEY_KP_EQUAL        ::ArcEngine::Key::KPEqual

#define ARC_KEY_LEFT_SHIFT      ::ArcEngine::Key::LeftShift
#define ARC_KEY_LEFT_CONTROL    ::ArcEngine::Key::LeftControl
#define ARC_KEY_LEFT_ALT        ::ArcEngine::Key::LeftAlt
#define ARC_KEY_LEFT_SUPER      ::ArcEngine::Key::LeftSuper
#define ARC_KEY_RIGHT_SHIFT     ::ArcEngine::Key::RightShift
#define ARC_KEY_RIGHT_CONTROL   ::ArcEngine::Key::RightControl
#define ARC_KEY_RIGHT_ALT       ::ArcEngine::Key::RightAlt
#define ARC_KEY_RIGHT_SUPER     ::ArcEngine::Key::RightSuper
#define ARC_KEY_MENU            ::ArcEngine::Key::Menu
