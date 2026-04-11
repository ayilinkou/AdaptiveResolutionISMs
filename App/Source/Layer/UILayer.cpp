#include <array>
#include <string>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "imgui.h"

#define UILAYER_CPP

#include "UILayer.h"
#include "AppLayer.h"
#include "Core/Application/Application.h"
#include "Core/Utility/Utility.h"
#include "Core/Light/LightManager.h"
#include "Core/Light/Light.h"
#include "Core/Renderer/RenderQueue.h"
#include "Scenes.h"

int UILayer::s_SelectedId = -1;

void UILayer::OnEvent(Core::Event& e)
{
	Core::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Core::KeyPressedEvent>([this](Core::KeyPressedEvent& e) { return OnKeyPressed(e); });
	dispatcher.Dispatch<Core::MouseMovedEvent>([this](Core::MouseMovedEvent& e) { return OnMouseMoved(e); });
}

void UILayer::OnUpdate(double dt)
{
	if (m_bVisible)
	{
		if (!StaticUtils::IsCursorVisible())
		{
			int Count;
			do {
				Count = ShowCursor(TRUE);
			} while (Count < 0);
		}
	}
	else
	{
		if (StaticUtils::IsCursorVisible())
		{
			int Count;
			do {
				Count = ShowCursor(FALSE);
			} while (Count >= 0);
		}
	}
}

void UILayer::OnRender(double dt)
{
	if (!m_bVisible)
		return;

	RenderMenuWindow();

	if (!m_bVisible)
		return;

	RenderLightingWindow();
}

bool UILayer::OnKeyPressed(Core::KeyPressedEvent& e)
{
	// consumes all inputs while visible
	switch (e.GetKeyCode())
	{
	case VK_ESCAPE:
		ToggleVisibility();
		return true;
	default:
		return m_bVisible;
	}
}

bool UILayer::OnMouseMoved(Core::MouseMovedEvent& e)
{
	return m_bVisible;
}

void UILayer::RenderMenuWindow()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

	ImVec2 buttonSize(200.f, 50.f);
	if (ImGui::Button("Load Emerald Square Night", buttonSize))
		LoadEmeraldSquareNight();

	if (ImGui::Button("Load Emerald Square Dusk", buttonSize))
		LoadEmeraldSquareDusk();

	if (ImGui::Button("Load Bistro Exterior", buttonSize))
		LoadBistroExterior();

	if (ImGui::Button("Load Bistro Interior", buttonSize))
		LoadBistroInterior();

	if (ImGui::Button("Load Bistro Interior Wine", buttonSize))
		LoadBistroInteriorWine();

	if (ImGui::Button("Quit", buttonSize))
		Core::Application::Get()->Stop();

	ImGui::End();
}

void UILayer::RenderLightingWindow()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 workPos = viewport->WorkPos;   // top-left of usable area
	ImVec2 workSize = viewport->WorkSize;  // usable size

	ImVec2 windowPos;
	windowPos.x = workPos.x + workSize.x;
	windowPos.y = workPos.y;
	
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2(1.f, 0.f));
	ImGui::Begin("Lighting", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

	auto camera = Core::Application::Get()->GetCamera();
	ImGui::Text("Camera");
	ImGui::PushID(0);
	ImGui::DragFloat3("Position", camera->GetPositionPtr());

	DirectX::XMFLOAT3 cameraRot = camera->GetTransform().Rotation;
	if (ImGui::DragFloat2("Rotation", reinterpret_cast<float*>(&cameraRot)))
		camera->SetRotation(cameraRot.x, cameraRot.y);

	ImGui::PopID();
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 2.f));

	std::string activeLightsStr("Active lights: ");
	UINT activeLightCount = 0u;
	for (Core::Light* pLight : Core::LightManager::GetLights())
	{
		if (pLight->IsActive())
			activeLightCount++;
	}
	activeLightsStr.append(std::to_string(activeLightCount));
	ImGui::Text(activeLightsStr.c_str());

	ImGui::SliderFloat("Ambient Strength", &Core::LightManager::GetAmbientStrengthRef(), 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::ColorEdit3("Skybox Color", reinterpret_cast<float*>(&Core::Renderer::Get()->GetClearColor()));
	
	ImGui::Dummy(ImVec2(0.f, 10.f));

	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	const char* shadowTypeStrings[] = { "Shadow Map", "ISM" };
	Core::ShadowType& shadowType = pAppLayer->GetShadowTypeRef();
	int currentShadowType = static_cast<int>(shadowType);
	if (ImGui::Combo("Shadow Type", &currentShadowType, shadowTypeStrings, IM_ARRAYSIZE(shadowTypeStrings)))
		shadowType = static_cast<Core::ShadowType>(currentShadowType);
	
	if (shadowType == Core::ShadowType::ISM)
	{
		ImGui::SliderFloat("Spot Light Min Shadow Bias", &Core::LightManager::GetSpotLightMinBiasISMRef(), 0.0001f, 0.001f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Spot Light Max Shadow Bias", &Core::LightManager::GetSpotLightMaxBiasISMRef(), 0.0001f, 0.001f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Splat World Radius", &Core::RenderQueue::GetISMSplatWorldRadiusRef(), 0.01f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Push Coverage Threshold", &Core::RenderQueue::GetISMCoverageThresholdRef(), 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	}
	else if (shadowType == Core::ShadowType::ShadowMap)
	{
		ImGui::SliderFloat("Spot Light Min Shadow Bias", &Core::LightManager::GetSpotLightMinBiasShadowMapRef(), 0.0001f, 0.001f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Spot Light Max Shadow Bias", &Core::LightManager::GetSpotLightMaxBiasShadowMapRef(), 0.0001f, 0.001f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	}

	ImGui::Separator();

	auto& lights = Core::LightManager::GetLights();
	ImGui::Text("Lights");
	ImGui::BeginChild("##", ImVec2(0, 250), ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar);
	for (int i = 0; i < lights.size(); i++)
	{
		ImGui::PushID(i);
		if (ImGui::Selectable(lights[i]->GetName().c_str(), s_SelectedId == i))
		{
			s_SelectedId = i;
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::Dummy(ImVec2(0.f, 10.f));

	if (s_SelectedId >= 0 && s_SelectedId < lights.size())
	{
		lights[s_SelectedId]->RenderControls();
	}

	ImGui::End();
}

void UILayer::ToggleVisibility()
{
	m_bVisible = !m_bVisible;
	if (m_bVisible)
	{
		Core::Application::Get()->RegisterNeedForCursor(this);
	}
	else
	{
		Core::Application::Get()->UnregisterNeedForCursor(this);
	}
}