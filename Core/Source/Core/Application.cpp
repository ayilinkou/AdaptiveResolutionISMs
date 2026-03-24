#include <ranges>

#include "Application.h"
#include "InputHandler.h"

namespace Core {
	Application* Application::s_pApp = nullptr;

	Application::Application(const ApplicationSpec& spec)
		: m_Spec(spec)
	{
		s_pApp = this;
		
		m_Window = std::make_shared<Window>(m_Spec.WinSpec);
		m_Window->Create();

		std::function<void(Event&)> callback = [this](Event& e) { RaiseEvent(e); };
		InputHandler::Init(callback);
	}

	Application::~Application()
	{
		InputHandler::Shutdown();
		m_Window->Destroy();
	}

	void Application::Run()
	{
		m_Running = true;

		m_AppStartTime = std::chrono::steady_clock::now();
		m_LastAppTime = m_AppStartTime;

		while (m_Running)
		{
			InputHandler::HandleInputs();

			if (m_Window->ShouldClose())
			{
				Stop();
				break;
			}

			UpdateAppTime();

			for (const std::unique_ptr<Layer>& layer : m_Layers)
				layer->OnUpdate(m_DeltaTime);

			for (const std::unique_ptr<Layer>& layer : m_Layers)
				layer->OnRender(m_DeltaTime);

			m_Window->Update();
		}
	}

	void Application::Stop()
	{
		m_Running = false;
	}

	void Application::RaiseEvent(Event& e)
	{
		for (auto& layer : std::views::reverse(m_Layers))
		{
			layer->OnEvent(e);
			if (e.bHandled)
				break;
		}
	}

	void Application::UpdateAppTime()
	{
		auto now = std::chrono::steady_clock::now();
		m_DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_LastAppTime).count() / 1000000.0; // in seconds
		m_LastAppTime = now;
	}
}