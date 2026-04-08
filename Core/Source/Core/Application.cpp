#include <ranges>

#include "Application.h"
#include "InputHandler.h"

namespace Core {
	Application* Application::s_pApp = nullptr;

	Application::Application(const ApplicationSpec& spec)
		: m_Spec(spec)
	{
		m_FPS = 0.0;
		m_DeltaTime = 0.0;
		m_AppTime = 0.0;

		s_pApp = this;
		
		HRESULT hResult;
		ASSERT_NOT_FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

		m_Window = std::make_shared<Window>(m_Spec.WinSpec);
		m_Window->Create();

		std::function<void(Event&)> callback = [this](Event& e) { RaiseEvent(e); };
		InputHandler::Init(callback, m_Window->GetHandle());

		m_Spec.RenderSpec.hwnd = m_Window->GetHandle();
		m_Renderer = std::make_shared<Renderer>(m_Spec.RenderSpec);
		m_Renderer->Init();

		m_ResourceManager = std::make_shared<ResourceManager>(m_Window->GetHandle(), m_Renderer->GetDevice());

		float fieldOfView = 3.141592654f / 4.f;
		float aspectRatio = (float)m_Spec.WinSpec.Width / (float)m_Spec.WinSpec.Height;
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, m_Spec.RenderSpec.NearPlane, m_Spec.RenderSpec.FarPlane);
		m_Camera = std::make_shared<Camera>(proj, m_Spec.RenderSpec.NearPlane, m_Spec.RenderSpec.FarPlane);
	}

	Application::~Application()
	{
		m_Layers.clear();
		
		m_ResourceManager.reset();
		m_Renderer.reset();
		InputHandler::Shutdown();
		m_Window.reset();
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

			std::string fpsAsString = std::to_string(m_FPS);
			SetWindowText(m_Window->GetHandle(), (m_Spec.Name + " - FPS: " + fpsAsString).c_str());

			m_Camera->CalcViewMatrix();
			Renderer::Get()->UpdateGlobalConstantBuffer(m_Camera.get(), (float)m_AppTime);

			for (const std::unique_ptr<Layer>& layer : m_Layers)
				layer->OnUpdate(m_DeltaTime);

			Renderer::Get()->BeginScene(0.3f, 0.6f, 0.8f, 1.f);

			for (const std::unique_ptr<Layer>& layer : m_Layers)
				layer->OnRender(m_DeltaTime);

			Renderer::Get()->EndScene();
		}
	}

	void Application::Stop()
	{
		m_Running = false;
	}

	void Application::RaiseEvent(Event& e)
	{
#if _DEBUG
		std::println("{}", e.ToString());
#endif
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
		m_FPS = 1.0 / m_DeltaTime;
		m_LastAppTime = now;
		m_AppTime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_AppStartTime).count() / 1000000.0;
	}
}