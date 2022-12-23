#pragma once

#include "Arc/Events/Event.h"
#include "Arc/Core/KeyCodes.h"

namespace ArcEngine
{
	class KeyEvent : public Event
	{
	public:
		[[nodiscard]] KeyCode GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		explicit KeyEvent(const KeyCode keyCode)
			:m_KeyCode(keyCode) {}

		KeyCode m_KeyCode;
	};
	
	class KeyPressedEvent : public KeyEvent
	{
	public:
		explicit KeyPressedEvent(const KeyCode keyCode, const uint16_t repeatCount)
			: KeyEvent(keyCode), m_RepeatCount(repeatCount) {}

		[[nodiscard]] uint16_t GetRepeatCount() const { return m_RepeatCount; }

		[[nodiscard]] std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		uint16_t m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		explicit KeyReleasedEvent(const KeyCode keyCode)
			: KeyEvent(keyCode) {}

		[[nodiscard]] std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		explicit KeyTypedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}

		[[nodiscard]] std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}

