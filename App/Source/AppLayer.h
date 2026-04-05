#pragma once

#include "Core/Layer.h"
#include "Core/InputEvents.h"

class AppLayer : public Core::Layer
{
public:
	AppLayer(const std::string& layerName) : Core::Layer(layerName) {}
	virtual ~AppLayer() {}

	virtual void OnEvent(Core::Event& e) override;

	virtual void OnUpdate(double dt) override;
	virtual void OnRender(double dt) override;

private:
	bool OnKeyPressed(Core::KeyPressedEvent& e);
};