#include "Application.h"

namespace Core {
	Application::Application(const ApplicationSpec& spec)
		: m_Spec(spec)
	{
		m_Window = std::make_shared<Window>(m_Spec.WinSpec);
		m_Window->Create();
	}

	Application::~Application()
	{
		m_Window->Destroy();
	}

	void Application::Run()
	{
		m_Running = true;

		while (m_Running)
		{
			if (m_Window->ShouldClose())
			{
				Stop();
				break;
			}

			m_Window->Update();
		}
	}

	void Application::Stop()
	{
		m_Running = false;
	}
}