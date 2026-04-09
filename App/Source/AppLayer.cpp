#include <print>

#include "AppLayer.h"
#include "Core/Application.h"
#include "Core/Renderer/Renderer.h"
#include "Core/ResourceManager.h"
#include "Core/MyMacros.h"
#include "Core/Shader/ShaderProgram.h"

#include "Core/Renderer/Model.h"
#include "Core/Renderer/ModelData.h"
#include "Core/Renderer/Texture.h"

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

	Core::ShaderProgramDesc desc;
	desc.Vertex.Filepath = "Source/Shaders/BasicVS.hlsl";
	desc.Pixel.Filepath = "Source/Shaders/BasicPS.hlsl";
	m_TestShaderProgram = std::make_unique<Core::ShaderProgram>(desc);

	D3D11_INPUT_ELEMENT_DESC vertexLayoutElements[3] = {};
	vertexLayoutElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayoutElements[0].SemanticName = "POSITION";
	vertexLayoutElements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexLayoutElements[0].AlignedByteOffset = 0;

	vertexLayoutElements[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayoutElements[1].SemanticName = "NORMAL";
	vertexLayoutElements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexLayoutElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

	vertexLayoutElements[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexLayoutElements[2].SemanticName = "TEXCOORD";
	vertexLayoutElements[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexLayoutElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

	UINT numElements = _countof(vertexLayoutElements);

	ASSERT_NOT_FAILED(device->CreateInputLayout(vertexLayoutElements, numElements, m_TestShaderProgram->GetVertexShaderBlob()->GetBufferPointer(),
		m_TestShaderProgram->GetVertexShaderBlob()->GetBufferSize(), &m_TestInputLayout));
	NAME_D3D_RESOURCE(m_TestInputLayout, "Test input layout");

	m_Sword = std::make_unique<Core::Model>("Models/fantasy_sword_stylized/scene.gltf", "Models/fantasy_sword_stylized");
}

void AppLayer::Shutdown()
{
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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->OMSetRenderTargets(1u, renderer->GetBackBufferRTV().GetAddressOf(), renderer->GetDSV().Get());

	context->IASetInputLayout(m_TestInputLayout.Get());
	context->VSSetShader(m_TestShaderProgram->GetVertexShader(), nullptr, 0u);
	context->PSSetShader(m_TestShaderProgram->GetPixelShader(), nullptr, 0u);
	m_Sword->GetModelData()->TestDraw();
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
