#pragma once

#include <vector>
#include <array>
#include <memory>

#include "Core/Layer/Layer.h"
#include "Core/Event/InputEvents.h"
#include "Core/Renderer/RenderQueue.h"
#include "Core/Model/Model.h"
#include "Core/Light/Light.h"

class AppLayer : public Core::Layer
{
public:
	AppLayer(const std::string& layerName);
	virtual ~AppLayer();

	virtual void OnEvent(Core::Event& e) override;

	virtual void OnUpdate(double dt) override;
	virtual void OnRender(double dt) override;

	void LoadScene(const std::string& modelPath, const std::string& texturesRoot);
	void LoadScenes(const std::vector<std::array<const std::string, 2>>& scenes);

	void AddLight(std::unique_ptr<Core::Light>&& light);

private:
	void Init();
	void Shutdown();

	bool OnKeyPressed(Core::KeyPressedEvent& e);
	bool OnMouseMoved(Core::MouseMovedEvent& e);

private:
	std::vector<std::unique_ptr<Core::Model>> m_Models;
	std::vector<std::unique_ptr<Core::Light>> m_Lights;

	std::unique_ptr<Core::RenderQueue> m_RenderQueue;
};