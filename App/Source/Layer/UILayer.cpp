#include <array>
#include <string>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "imgui.h"

#include "UILayer.h"
#include "Core/Application/Application.h"
#include "Core/Utility/Utility.h"
#include "AppLayer.h"

namespace ScenePaths {
	std::array<std::string, 2> EmeraldSquareDay = { "Models/EmeraldSquare_v4_1/EmeraldSquare_Day.fbx", "Models/EmeraldSquare_v4_1" };
	std::array<std::string, 2> EmeraldSquareDusk = { "Models/EmeraldSquare_v4_1/EmeraldSquare_Dusk.fbx", "Models/EmeraldSquare_v4_1" };
	std::array<std::string, 2> BistroExterior = { "Models/Bistro_v5_2/BistroExterior.fbx", "Models/Bistro_v5_2" };
	std::array<std::string, 2> BistroInterior = { "Models/Bistro_v5_2/BistroInterior.fbx", "Models/Bistro_v5_2" };
	std::array<std::string, 2> BistroInteriorWine = { "Models/Bistro_v5_2/BistroInterior_Wine.fbx", "Models/Bistro_v5_2" };
}

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

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

	ImVec2 buttonSize(200.f, 50.f);
	if (ImGui::Button("Load Emerald Square Day", buttonSize))
	{
		AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
		if (pAppLayer)
			pAppLayer->LoadScene(ScenePaths::EmeraldSquareDay[0], ScenePaths::EmeraldSquareDay[1]);
		ToggleVisibility();
	}

	if (ImGui::Button("Load Emerald Square Dusk", buttonSize))
	{
		AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
		if (pAppLayer)
			pAppLayer->LoadScene(ScenePaths::EmeraldSquareDusk[0], ScenePaths::EmeraldSquareDusk[1]);
		ToggleVisibility();
	}

	if (ImGui::Button("Load Bistro Exterior", buttonSize))
	{
		AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
		if (pAppLayer)
			pAppLayer->LoadScene(ScenePaths::BistroExterior[0], ScenePaths::BistroExterior[1]);
		ToggleVisibility();
	}

	if (ImGui::Button("Load Bistro Interior", buttonSize))
	{
		AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
		if (pAppLayer)
			pAppLayer->LoadScene(ScenePaths::BistroInterior[0], ScenePaths::BistroInterior[1]);
		ToggleVisibility();
	}

	if (ImGui::Button("Load Bistro Interior Wine", buttonSize))
	{
		AppLayer* pAppLayer = Core::Application::Get()->GetLayer<AppLayer>();
		if (pAppLayer)
			pAppLayer->LoadScene(ScenePaths::BistroInteriorWine[0], ScenePaths::BistroInteriorWine[1]);
		ToggleVisibility();
	}

	if (ImGui::Button("Quit", buttonSize))
	{
		Core::Application::Get()->Stop();
	}
	ImGui::End();
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