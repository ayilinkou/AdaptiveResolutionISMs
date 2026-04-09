#pragma once

#include "wrl.h"

#include "d3d11.h"
#include "DirectXMath.h"

#include "Core/Layer/Layer.h"
#include "Core/Event/InputEvents.h"
#include "Core/Shader/ShaderProgram.h"
#include "Core/Renderer/RenderQueue.h"
#include "Core/Model/Model.h"

using namespace Microsoft::WRL;

class AppLayer : public Core::Layer
{
public:
	AppLayer(const std::string& layerName);
	virtual ~AppLayer();

	virtual void OnEvent(Core::Event& e) override;

	virtual void OnUpdate(double dt) override;
	virtual void OnRender(double dt) override;

private:
	void Init();
	void Shutdown();

	bool OnKeyPressed(Core::KeyPressedEvent& e);
	bool OnMouseMoved(Core::MouseMovedEvent& e);

	void ApplyCameraMovement();

private:
	std::unique_ptr<Core::Model> m_EmeraldSquare;
	std::unique_ptr<Core::Model> m_Bistro;

	std::unique_ptr<Core::RenderQueue> m_RenderQueue;

	UINT m_IndexCount;
};