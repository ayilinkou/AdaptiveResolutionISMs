#pragma once

#include "Core/Layer/Layer.h"
#include "Core/Event/InputEvents.h"

class AppLayer;

namespace Core {
	enum class ShadowMethod;
}

class UILayer : public Core::Layer
{
public:
	UILayer(const std::string& layerName)
		: Layer(layerName) { ToggleVisibility(); }

	virtual void OnEvent(Core::Event& e) override;

	virtual void OnUpdate(double dt) override;
	virtual void OnRender(double dt) override;

protected:
	bool m_bVisible = false;

private:
	bool OnKeyPressed(Core::KeyPressedEvent& e);
	bool OnMouseMoved(Core::MouseMovedEvent& e);

	void RenderScenesWindow();
	void RenderSettingsWindow();

	inline void RenderSMSettings(AppLayer* pAppLayer, Core::ShadowMethod shadowMethod);
	inline void RenderISMSettings(AppLayer* pAppLayer, Core::ShadowMethod shadowMethod);
	inline void RenderLowISMSettings(AppLayer* pAppLayer, Core::ShadowMethod shadowMethod);

	void ToggleVisibility();

	void LoadEmeraldSquareNight();
	void LoadBistroExterior();
	void LoadFromFile();

private:
	static int s_SelectedId;
};