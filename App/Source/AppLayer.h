#pragma once

#include "wrl.h"

#include "d3d11.h"
#include "DirectXMath.h"

#include "Core/Layer.h"
#include "Core/InputEvents.h"
#include "Core/Shader/ShaderProgram.h"

#include "Core/Renderer/Model.h" // temp
#include "Core/Renderer/Texture.h" // temp

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
	ComPtr<ID3D11InputLayout> m_TestInputLayout;

	std::unique_ptr<Core::ShaderProgram> m_TestShaderProgram;

	std::unique_ptr<Core::Model> m_Sword;


	UINT m_IndexCount;
};