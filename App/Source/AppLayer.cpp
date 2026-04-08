#include <print>

#include "AppLayer.h"
#include "Core/Application.h"
#include "Core/Renderer.h"
#include "Core/ResourceManager.h"
#include "Core/MyMacros.h"

AppLayer::AppLayer(const std::string& layerName)
	: Core::Layer(layerName)
{
	Init();
}

AppLayer::~AppLayer()
{
	Shutdown();
}

void AppLayer::Init()
{
	HRESULT hResult;
	ID3D11DeviceContext* context = Core::Renderer::Get()->GetContext().Get();
	ID3D11Device* device = Core::Renderer::Get()->GetDevice().Get();

	UINT indices[] = {
		0, 1, 2
	};

	m_IndexCount = _countof(indices);

	float vertices[] = {
		 0.0f,  0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f
	};

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = &indices;

	ASSERT_NOT_FAILED(device->CreateBuffer(&indexBufferDesc, &data, &m_IndexBuffer));
	NAME_D3D_RESOURCE(m_IndexBuffer, "Test index buffer");

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	data.pSysMem = &vertices;

	ASSERT_NOT_FAILED(device->CreateBuffer(&vertexBufferDesc, &data, &m_VertexBuffer));
	NAME_D3D_RESOURCE(m_VertexBuffer, "Test vertex buffer");

	ComPtr<ID3D10Blob> vsBuffer;
	m_vShaderPath = "Source/Shaders/tri.hlsl";
	m_pShaderPath = "Source/Shaders/basic.hlsl";
	m_VertexShader = Core::ResourceManager::Get()->LoadShader<ID3D11VertexShader>(m_vShaderPath, "main", vsBuffer);
	m_PixelShader = Core::ResourceManager::Get()->LoadShader<ID3D11PixelShader>(m_pShaderPath);

	D3D11_INPUT_ELEMENT_DESC vertexLayout[1] = {};
	vertexLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[0].SemanticName = "POSITION";
	vertexLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	UINT numElements = _countof(vertexLayout);

	ASSERT_NOT_FAILED(device->CreateInputLayout(vertexLayout, numElements, vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_InputLayout));
	NAME_D3D_RESOURCE(m_InputLayout, "Test input layout");
}

void AppLayer::Shutdown()
{
	Core::ResourceManager::Get()->UnloadShader<ID3D11VertexShader>(m_vShaderPath);
	Core::ResourceManager::Get()->UnloadShader<ID3D11PixelShader>(m_pShaderPath);
}

void AppLayer::OnEvent(Core::Event& event)
{
	Core::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Core::KeyPressedEvent>([this](Core::KeyPressedEvent& e) { return OnKeyPressed(e); });
	dispatcher.Dispatch<Core::MouseMovedEvent>([this](Core::MouseMovedEvent& e) { return OnMouseMoved(e); });
}

void AppLayer::OnUpdate(double dt)
{
	ApplyCameraMovement();
}

void AppLayer::OnRender(double dt)
{
	Core::Renderer* renderer = Core::Renderer::Get();
	ID3D11DeviceContext* context = renderer->GetContext().Get();

	UINT strides[] = { sizeof(float) * 3u };
	UINT offsets[] = { 0u };

	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	context->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), strides, offsets);
	context->IASetInputLayout(m_InputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(m_VertexShader, nullptr, 0u);

	context->PSSetShader(m_PixelShader, nullptr, 0u);

	context->OMSetRenderTargets(1u, renderer->GetBackBufferRTV().GetAddressOf(), renderer->GetDSV().Get());

	context->DrawIndexed(m_IndexCount, 0u, 0);
}

bool AppLayer::OnKeyPressed(Core::KeyPressedEvent& e)
{
	// These key presses include the delay before repeating. For smooth keyboard inputs, use GetAsyncKeyState() instead.
	switch (e.GetKeyCode())
	{
	case VK_ESCAPE:
		Core::Application::Get()->Stop(); // exit application when ESC is pressed
		return true;
	default:
		return false;
	}
}

bool AppLayer::OnMouseMoved(Core::MouseMovedEvent& e)
{
	Core::Application::Get()->GetCamera()->RotateCamera((float)e.GetX(), (float)e.GetY());
	return true;
}

void AppLayer::ApplyCameraMovement()
{	
	// bit 0x8000 signals if the key is currently down
	DirectX::XMFLOAT3 movementInputVector = {};
	float qeVector = 0.f;
	if (GetAsyncKeyState('W') & 0x8000)
		movementInputVector.z += 1.f;
	if (GetAsyncKeyState('S') & 0x8000)
		movementInputVector.z -= 1.f;
	if (GetAsyncKeyState('A') & 0x8000)
		movementInputVector.x -= 1.f;
	if (GetAsyncKeyState('D') & 0x8000)
		movementInputVector.x += 1.f;
	if (GetAsyncKeyState('Q') & 0x8000)
		qeVector -= 1.f;
	if (GetAsyncKeyState('E') & 0x8000)
		qeVector += 1.f;

	constexpr DirectX::XMFLOAT3 zeroVector = {};
	if (memcmp(&zeroVector, &movementInputVector, sizeof(DirectX::XMFLOAT3)) == 0 && qeVector == 0.f)
		return;
	
	Core::Application::Get()->GetCamera()->MoveCamera(movementInputVector, qeVector);
	movementInputVector = {};
}
