#pragma once

#include <string>
#include <memory>

#include "Window.h"

namespace Core {
	struct ApplicationSpec
	{
		std::string Name = "Application";
		WindowSpec WinSpec;
	};

	class Application
	{
	public:
		Application() = delete;
		Application(const ApplicationSpec& spec = ApplicationSpec());
		~Application();

		void Run();
	
	private:
		void Stop();

	private:
		ApplicationSpec m_Spec;
		std::shared_ptr<Window> m_Window;

		bool m_Running = false;
	};
}