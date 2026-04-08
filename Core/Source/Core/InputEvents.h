#pragma once

#include <format>

#include "Event.h"

namespace Core {
	class KeyEvent : public Event
	{
	public:
		int GetKeyCode() const { return m_KeyCode; }

	protected:
		KeyEvent(int keyCode)
			: m_KeyCode(keyCode) {}
	
		int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int keyCode, bool bIsRepeat)
			: KeyEvent(keyCode), m_bIsRepeat(bIsRepeat) {}
	
		bool IsRepeat() const { return m_bIsRepeat; }

		std::string ToString() const override
		{
			return std::format("KeyPressedEvent: {} (repeat={})", m_KeyCode, m_bIsRepeat);
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		bool m_bIsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keyCode)
			: KeyEvent(keyCode) {}
	
		std::string ToString() const override
		{
			return std::format("KeyReleasedEvent: {}", m_KeyCode);
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(double x, double y)
			: m_MouseX(x), m_MouseY(y) {
		}

		double GetX() const { return m_MouseX; }
		double GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			return std::format("MouseMovedEvent: {}, {}", m_MouseX, m_MouseY);
		}

		EVENT_CLASS_TYPE(MouseMoved)

	private:
		double m_MouseX;
		double m_MouseY;
	};
}