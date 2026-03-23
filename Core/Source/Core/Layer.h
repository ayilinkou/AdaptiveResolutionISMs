#pragma once

#include <memory>

#include "Event.h"

namespace Core {
	class Layer
	{
	public:
		virtual ~Layer() = default;

		virtual void OnEvent(Event& e) {}

		virtual void OnUpdate(double dt) {}
		virtual void OnRender(double dt) {}
	};
}