#pragma once

#include <string>
#include <memory>
#include <vector>
#include <chrono>

#include "Window.h"
#include "Layer.h"

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
		void Stop();

	private:
		template<typename TLayer>
			requires(std::is_base_of_v<Layer, TLayer>)
		void PushLayer()
		{
			m_Layers.push_back(std::make_unique<TLayer>());
		}

		template<typename TLayer>
			requires(std::is_base_of_v<Layer, TLayer>)
		TLayer* GetLayer()
		{
			for (const auto& layer : m_Layers)
			{
				if (auto casted = dynamic_cast<TLayer*>(layer.get()))
					return casted;
			}
			return nullptr;
		}

		void RaiseEvent(Event& e);

		void UpdateAppTime();

	private:
		ApplicationSpec m_Spec;
		std::shared_ptr<Window> m_Window;
		std::vector<std::unique_ptr<Layer>> m_Layers;

		std::chrono::steady_clock::time_point m_AppStartTime;
		std::chrono::steady_clock::time_point m_LastAppTime;
		double m_DeltaTime;
		bool m_Running = false;
	};
}