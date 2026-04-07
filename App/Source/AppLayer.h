#pragma once

#include "wrl.h"

#include "d3d11.h"

#include "Core/Layer.h"
#include "Core/InputEvents.h"

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

private:
	ComPtr<ID3D11Buffer> m_IndexBuffer;
	ComPtr<ID3D11Buffer> m_VertexBuffer;
	ComPtr<ID3D11InputLayout> m_InputLayout;

	std::string m_vShaderPath;
	std::string m_pShaderPath;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

	UINT m_IndexCount;
};