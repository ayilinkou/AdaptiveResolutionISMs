#include "AppLayer.h"
#include "Scenes.h"
#include "Core/Application/Application.h"
#include "Core/Renderer/Renderer.h"
#include "Core/Utility/Timer.h"
#include "Core/Light/LightManager.h"

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
	m_RenderQueue = std::make_unique<Core::RenderQueue>();
	m_RenderQueue->SetSMCount((UINT)m_SMCount);
	m_RenderQueue->SetISMCount((UINT)m_ISMCount);
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
	Core::Application::Get()->GetCamera()->MoveCamera((float)dt);
	
	m_RenderQueue->SetSMCount((UINT)m_SMCount);
	m_RenderQueue->SetISMCount((UINT)m_ISMCount);
	m_RenderQueue->PopulateRenderQueue();
}

void AppLayer::OnRender(double dt)
{	
	{
		//Timer timer("RenderGeometryPass()");
		m_RenderQueue->RenderGeometryPass();
	}
	
	{
		//Timer timer("RenderShadowPass()");
		m_RenderQueue->RenderShadowPass(m_ShadowMethod);
	}

	{
		//Timer timer("RenderLightingPass()");
		m_RenderQueue->RenderLightingPass(m_ShadowMethod);
	}
}

void AppLayer::LoadScene(const SceneInfo& scene)
{
	Core::Renderer* pRenderer = Core::Renderer::Get();
	UnloadScene();

	scene.PointCloudDensity > 0.f ? pRenderer->SetPointCloudDensity(scene.PointCloudDensity) : pRenderer->ResetPointCloudDensity();

	float minBias = scene.MinBiasShadowMap > 0.f ? scene.MinBiasShadowMap : Core::LightManager::GetSpotLightMinBiasShadowMapRef();
	float maxBias = scene.MaxBiasShadowMap > 0.f ? scene.MaxBiasShadowMap : Core::LightManager::GetSpotLightMaxBiasShadowMapRef();
	Core::LightManager::SetSpotLightBiasShadowMap(minBias, maxBias);

	minBias = scene.MinBiasISM > 0.f ? scene.MinBiasISM : Core::LightManager::GetSpotLightMinBiasISMRef();
	maxBias = scene.MaxBiasISM > 0.f ? scene.MaxBiasISM : Core::LightManager::GetSpotLightMaxBiasISMRef();
	Core::LightManager::SetSpotLightBiasISM(minBias, maxBias);

	minBias = scene.MinBiasLowISM > 0.f ? scene.MinBiasLowISM : Core::LightManager::GetSpotLightMinBiasLowISMRef();
	maxBias = scene.MaxBiasLowISM > 0.f ? scene.MaxBiasLowISM : Core::LightManager::GetSpotLightMaxBiasLowISMRef();
	Core::LightManager::SetSpotLightBiasLowISM(minBias, maxBias);

	m_Models.emplace_back(std::make_unique<Core::Model>(scene.ModelPath, scene.TexturesRoot));
}

void AppLayer::UnloadScene()
{
	Core::Renderer::Get()->ResetClearColor();
	m_Models.clear();
	m_Lights.clear();
}

void AppLayer::AddLight(std::unique_ptr<Core::Light>&& light)
{
	m_Lights.push_back(std::move(light));
}

bool AppLayer::OnKeyPressed(Core::KeyPressedEvent& e)
{
	switch (e.GetKeyCode())
	{
	case VK_ESCAPE:
	{
		// exit application when ESC is pressed
		// only reachable if no UILayer exists
		// otherwise can ESC will open the UI with a quit button
		Core::Application::Get()->Stop();
		return true;
	}
	case 'W':
	{
		if (!m_bMovementEnabled)
			return true;
		Core::Application::Get()->GetCamera()->AddWASDVector({ 0.f,  0.f,  1.f });
		return true;
	}
	case 'S':
	{
		if (!m_bMovementEnabled)
			return true;
		Core::Application::Get()->GetCamera()->AddWASDVector({  0.f,  0.f, -1.f });
		return true;
	}
	case 'A':
	{
		if (!m_bMovementEnabled)
			return true;
		Core::Application::Get()->GetCamera()->AddWASDVector({ -1.f,  0.f,  0.f });
		return true;
	}
	case 'D':
	{
		if (!m_bMovementEnabled)
			return true;
		Core::Application::Get()->GetCamera()->AddWASDVector({  1.f,  0.f,  0.f });
		return true;
	}
	case 'Q':
	{
		if (!m_bMovementEnabled)
			return true;
		Core::Application::Get()->GetCamera()->AddQEVector(-1.f);
		return true;
	}
	case 'E':
	{
		if (!m_bMovementEnabled)
			return true;
		Core::Application::Get()->GetCamera()->AddQEVector( 1.f);
		return true;
	}
	case 'M':
	{
		m_bMovementEnabled = !m_bMovementEnabled;
		return true;
	}
	case 'F':
	{
		std::cout << Core::Application::Get()->GetAverageFrameTime() << std::endl;
		return true;
	}
	default:
		return false;
	}
}

bool AppLayer::OnMouseMoved(Core::MouseMovedEvent& e)
{
	if (!m_bMovementEnabled)
		return true;
	Core::Application::Get()->GetCamera()->RotateCamera((float)e.GetX(), (float)e.GetY());
	return true;
}
