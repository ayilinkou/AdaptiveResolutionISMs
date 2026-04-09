#include <print>

#include "AppLayer.h"
#include "Core/Application/Application.h"
#include "Core/Renderer/Renderer.h"
#include "Core/Resource/ResourceManager.h"

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
	m_EmeraldSquare = std::make_unique<Core::Model>("Models/EmeraldSquare_v4_1/EmeraldSquare_Day.fbx", "Models/EmeraldSquare_v4_1");
	//m_EmeraldSquare = std::make_unique<Core::Model>("Models/EmeraldSquare_v4_1/EmeraldSquare_Dusk.fbx", "Models/EmeraldSquare_v4_1");
	//m_Bistro = std::make_unique<Core::Model>("Models/Bistro_v5_2/BistroExterior.fbx", "Models/Bistro_v5_2");
	//m_Bistro = std::make_unique<Core::Model>("Models/Bistro_v5_2/BistroInterior.fbx", "Models/Bistro_v5_2");
	//m_Bistro = std::make_unique<Core::Model>("Models/Bistro_v5_2/BistroInterior_Wine.fbx", "Models/Bistro_v5_2");

	m_RenderQueue = std::make_unique<Core::RenderQueue>();
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

	m_RenderQueue->PopulateRenderQueue();
}

void AppLayer::OnRender(double dt)
{
	Core::Renderer* renderer = Core::Renderer::Get();
	ID3D11DeviceContext* context = renderer->GetContext().Get();

	context->OMSetRenderTargets(1u, renderer->GetBackBufferRTV().GetAddressOf(), renderer->GetDSV().Get());

	m_RenderQueue->Render();
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
