#pragma once

#include "Core/Layer/Layer.h"
#include "Core/Event/InputEvents.h"

class UILayer : public Core::Layer
{
public:
	UILayer(const std::string& layerName)
		: Layer(layerName) {}

	virtual void OnEvent(Core::Event& e) override;

	virtual void OnUpdate(double dt) override;
	virtual void OnRender(double dt) override;

protected:
	bool m_bVisible = false;

private:
	bool OnKeyPressed(Core::KeyPressedEvent& e);
	bool OnMouseMoved(Core::MouseMovedEvent& e);

	void RenderMenuWindow();
	void RenderLightingWindow();

	void ToggleVisibility();

	void LoadEmeraldSquareNight();
	void LoadEmeraldSquareDusk();
	void LoadBistroExterior();
	void LoadBistroInterior();
	void LoadBistroInteriorWine();

private:
	static int s_SelectedId;
};