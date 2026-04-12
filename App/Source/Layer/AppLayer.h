#pragma once

#include <vector>
#include <memory>

#include "Core/Layer/Layer.h"
#include "Core/Event/InputEvents.h"
#include "Core/Renderer/RenderQueue.h"
#include "Core/Model/Model.h"
#include "Core/Light/Light.h"

struct SceneInfo;

class AppLayer : public Core::Layer
{
public:
	AppLayer(const std::string& layerName);
	virtual ~AppLayer();

	virtual void OnEvent(Core::Event& e) override;

	virtual void OnUpdate(double dt) override;
	virtual void OnRender(double dt) override;

	void LoadScene(const SceneInfo& scene);

	void AddLight(std::unique_ptr<Core::Light>&& light);

	Core::ShadowMethod& GetShadowMethodRef() { return m_ShadowMethod; }
	int& GetSMCountRef() { return m_SMCount; }
	int& GetISMCountRef() { return m_ISMCount; }

private:
	void Init();
	void Shutdown();

	bool OnKeyPressed(Core::KeyPressedEvent& e);
	bool OnMouseMoved(Core::MouseMovedEvent& e);

private:
	std::vector<std::unique_ptr<Core::Model>> m_Models;
	std::vector<std::unique_ptr<Core::Light>> m_Lights;

	std::unique_ptr<Core::RenderQueue> m_RenderQueue;

	Core::ShadowMethod m_ShadowMethod = Core::ShadowMethod::AdaptiveISM;

	int m_SMCount = 1;
	int m_ISMCount = 1;
};