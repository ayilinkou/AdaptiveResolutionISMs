#pragma once

#include <string>
#include <functional>

namespace Core {
	enum class EventType
	{
		None = 0,
		KeyPressed,
		KeyReleased
	};


#define EVENT_CLASS_TYPE(type)  static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

	class Event
	{
	public:
		virtual ~Event() {}
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }

	public:
		bool bHandled = false;
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& e)
			: m_Event(e) {}

		template<typename T>
		bool Dispatch(std::function<bool(T&)> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.bHandled)
			{
				m_Event.bHandled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};
}