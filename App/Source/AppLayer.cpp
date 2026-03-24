#include <print>

#include "AppLayer.h"
#include "Core/Application.h"

void AppLayer::OnEvent(Core::Event& event)
{
	std::println("{}", event.ToString());

	Core::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Core::KeyPressedEvent>([this](Core::KeyPressedEvent& e) { return OnKeyPressed(e); });
}

void AppLayer::OnUpdate(double dt)
{
}

void AppLayer::OnRender(double dt)
{
}

bool AppLayer::OnKeyPressed(Core::KeyPressedEvent& e)
{
	switch (e.GetKeyCode())
	{
	case VK_ESCAPE:
		Core::Application::Get()->Stop(); // exit application when ESC is pressed
		return true;
	default:
		return false;
	}
}
