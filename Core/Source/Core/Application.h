#pragma once

#include <string>

namespace Core {
	struct ApplicationSpec
	{
		std::string Name = "Application";
	};

	class Application
	{
	public:
		Application() = delete;
		Application(const ApplicationSpec& spec = ApplicationSpec());
		~Application();

	private:
		ApplicationSpec m_Spec;
	};
}