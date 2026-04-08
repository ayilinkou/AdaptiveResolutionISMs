#pragma once

#include <string>
#include <memory>
#include <vector>
#include <chrono>

#include "Window.h"
#include "Layer.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Camera.h"

namespace Core {
	struct ApplicationSpec
	{
		std::string Name = "Application";
		WindowSpec WinSpec;
		RendererSpec RenderSpec;
	};

	class Application
	{
	public:
		Application() = delete;
		Application(const ApplicationSpec& spec = ApplicationSpec());
		~Application();

		void Run();
		void Stop();

		static Application* Get() { return s_pApp; }

		template<typename TLayer, typename... Args>
			requires(std::is_base_of_v<Layer, TLayer>)
		void PushLayer(Args&&... args)
		{
			m_Layers.push_back(std::make_unique<TLayer>(std::forward<Args>(args)...));
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

		std::shared_ptr<Camera> GetCamera() { return m_Camera; }
		double GetDeltaTime() const { return m_DeltaTime; }

	private:
		void RaiseEvent(Event& e);

		void UpdateAppTime();

	private:
		ApplicationSpec m_Spec;
		std::shared_ptr<Window> m_Window;
		std::shared_ptr<Renderer> m_Renderer;
		std::shared_ptr<ResourceManager> m_ResourceManager;
		std::shared_ptr<Camera> m_Camera;
		std::vector<std::unique_ptr<Layer>> m_Layers;

		std::chrono::steady_clock::time_point m_AppStartTime;
		std::chrono::steady_clock::time_point m_LastAppTime;
		double m_AppTime;
		double m_DeltaTime;
		double m_FPS;
		bool m_Running = false;

		static Application* s_pApp;
	};
}