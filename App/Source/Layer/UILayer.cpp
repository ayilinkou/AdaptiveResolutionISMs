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
		if (!Core::StaticUtils::IsCursorVisible())
		{
			int Count;
			do {
				Count = ShowCursor(TRUE);
			} while (Count < 0);
		}
	}
	else
	{
		if (Core::StaticUtils::IsCursorVisible())
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

	RenderScenesWindow();

	if (!m_bVisible)
		return;

	RenderSettingsWindow();
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

void UILayer::RenderScenesWindow()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::Begin("Scenes", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

	ImVec2 buttonSize(200.f, 50.f);
	if (ImGui::Button("Load Emerald Square", buttonSize))
		LoadEmeraldSquare();

	if (ImGui::Button("Load Bistro Exterior", buttonSize))
		LoadBistroExterior();

	if (ImGui::Button("Load San Miguel", buttonSize))
		LoadSanMiguel();

	if (ImGui::Button("Load scene from file...", buttonSize))
		LoadFromFile();

	if (ImGui::Button("Unload Scene", buttonSize))
	{
		AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
		pAppLayer->UnloadScene();
	}

	if (ImGui::Button("Quit", buttonSize))
		Core::Application::Get()->Stop();

	ImGui::End();
}

void UILayer::RenderSettingsWindow()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 workPos = viewport->WorkPos;   // top-left of usable area
	ImVec2 workSize = viewport->WorkSize;  // usable size

	ImVec2 windowPos;
	windowPos.x = workPos.x + workSize.x;
	windowPos.y = workPos.y;
	
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2(1.f, 0.f));
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

	AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
	auto camera = Core::Application::Get()->GetCamera();
	ImGui::Text("Camera");

	constexpr const char* movementEnabledStr = "Movement enabled";
	constexpr const char* movementDisabledStr = "Movement disabled";

	ImGui::Text(pAppLayer->GetMovementEnabled() ? movementEnabledStr : movementDisabledStr);
	ImGui::PushID(0);
	ImGui::DragFloat3("Position", camera->GetPositionPtr());

	DirectX::XMFLOAT3 cameraRot = camera->GetTransform().Rotation;
	if (ImGui::DragFloat2("Rotation", reinterpret_cast<float*>(&cameraRot)))
		camera->SetRotation(cameraRot.x, cameraRot.y);

	ImGui::PopID();
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 2.f));

	ImGui::Text("Environment");
	ImGui::SliderFloat("Ambient Strength", &Core::LightManager::GetAmbientStrengthRef(), 0.f, 0.1f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::ColorEdit3("Skybox Color", reinterpret_cast<float*>(&Core::Renderer::Get()->GetClearColor()));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 2.f));

	ImGui::Text("Shadows");
	const char* shadowMethodStrings[] = { "Shadow Maps", "Static ISMs", "Adaptive ISMs"};
	Core::ShadowMethod& shadowMethod = pAppLayer->GetShadowMethodRef();
	int currentShadowMethod = static_cast<int>(shadowMethod);
	if (ImGui::Combo("Shadowing Method", &currentShadowMethod, shadowMethodStrings, IM_ARRAYSIZE(shadowMethodStrings)))
		shadowMethod = static_cast<Core::ShadowMethod>(currentShadowMethod);
	
	ImGui::Dummy(ImVec2(0.f, 10.f));
	RenderSMSettings(pAppLayer, shadowMethod);

	if (shadowMethod == Core::ShadowMethod::StaticISM)
	{
		RenderISMSettings(pAppLayer, shadowMethod);
	}
	else if (shadowMethod == Core::ShadowMethod::AdaptiveISM)
	{
		RenderLowISMSettings(pAppLayer, shadowMethod);
	}

	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 2.f));

	auto& lights = Core::LightManager::GetLights();
	ImGui::Text("Lights");

	std::string activeLightsStr("Active lights: ");
	UINT activeLightCount = 0u;
	for (Core::Light* pLight : Core::LightManager::GetLights())
	{
		if (pLight->IsActive())
			activeLightCount++;
	}
	activeLightsStr.append(std::to_string(activeLightCount));
	ImGui::Text(activeLightsStr.c_str());

	ImGui::BeginChild("##", ImVec2(0, 250), ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar);
	for (int i = 0; i < lights.Size(); i++)
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

	if (s_SelectedId >= 0 && s_SelectedId < lights.Size())
	{
		lights[s_SelectedId]->RenderControls();
	}

	ImGui::End();
}

void UILayer::RenderSMSettings(AppLayer* pAppLayer, Core::ShadowMethod shadowMethod)
{
	ImGui::Text("Shadow Map");
	if (shadowMethod != Core::ShadowMethod::ShadowMap)
	{
		ImGui::SliderInt("Max Shadow Map Count", &pAppLayer->GetSMCountRef(), 0, MAX_SPOT_LIGHT_COUNT, "%d", ImGuiSliderFlags_AlwaysClamp);
	}
	ImGui::SliderFloat("Min Shadow Bias##SM", &Core::LightManager::GetSpotLightMinBiasShadowMapRef(), 0.0001f, 0.005f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Max Shadow Bias##SM", &Core::LightManager::GetSpotLightMaxBiasShadowMapRef(), 0.0001f, 0.005f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
}

void UILayer::RenderISMSettings(AppLayer* pAppLayer, Core::ShadowMethod shadowMethod)
{
	ImGui::Dummy(ImVec2(0.f, 2.f));
	ImGui::Text("Standard ISM");
	if (shadowMethod == Core::ShadowMethod::AdaptiveISM)
	{
		ImGui::SliderInt("Max Standard ISM Count", &pAppLayer->GetISMCountRef(), 0, MAX_SPOT_LIGHT_COUNT, "%d", ImGuiSliderFlags_AlwaysClamp);
	}
	ImGui::SliderFloat("Min Shadow Bias##ISM", &Core::LightManager::GetSpotLightMinBiasISMRef(), 0.0001f, 0.01f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Max Shadow Bias##ISM", &Core::LightManager::GetSpotLightMaxBiasISMRef(), 0.0001f, 0.01f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Splat World Radius##ISM", &Core::RenderQueue::GetISMSplatWorldRadiusRef(), 0.01f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Push Coverage Threshold##ISM", &Core::RenderQueue::GetISMCoverageThresholdRef(), 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
}

void UILayer::RenderLowISMSettings(AppLayer* pAppLayer, Core::ShadowMethod shadowMethod)
{
	RenderISMSettings(pAppLayer, shadowMethod);
	ImGui::Dummy(ImVec2(0.f, 2.f));
	ImGui::Text("Low Res ISM");
	ImGui::SliderFloat("Min Shadow Bias##LowISM", &Core::LightManager::GetSpotLightMinBiasLowISMRef(), 0.0001f, 0.01f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Max Shadow Bias##LowISM", &Core::LightManager::GetSpotLightMaxBiasLowISMRef(), 0.0001f, 0.01f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Splat World Radius##LowISM", &Core::RenderQueue::GetLowISMSplatWorldRadiusRef(), 0.01f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Push Coverage Threshold##LowISM", &Core::RenderQueue::GetLowISMCoverageThresholdRef(), 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
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