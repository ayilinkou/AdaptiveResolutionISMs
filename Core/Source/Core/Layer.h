#pragma once

#include <memory>
#include <string>
#include <print>

#include "Event.h"

namespace Core {
	class Layer
	{
	public:
		Layer(const std::string& layerName)
			: m_LayerName(layerName) { std::println("Created new layer: {}", m_LayerName); }

		virtual ~Layer() = default;

		virtual void OnEvent(Event& e) = 0;

		virtual void OnUpdate(double dt) = 0;
		virtual void OnRender(double dt) = 0;
		
		const std::string& GetName() const { return m_LayerName; }

	protected:
		const std::string m_LayerName;
	};
}