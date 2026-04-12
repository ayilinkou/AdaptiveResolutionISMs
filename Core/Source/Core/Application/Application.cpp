#include <ranges>
#include <thread>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "Application.h"
#include "Core/Input/InputHandler.h"
#include "Core/Utility/Logger.h"
#include "Core/Utility/Timer.h"
#include "Core/Light/LightManager.h"

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

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		m_Window = std::make_shared<Window>(m_Spec.WinSpec);
		m_Window->Init();

		Logger::Init(m_Window->GetHandle());

		std::function<void(Event&)> callback = [this](Event& e) { RaiseEvent(e); };
		InputHandler::Init(callback, m_Window->GetHandle());

		m_Spec.RenderSpec.hwnd = m_Window->GetHandle();
		m_Renderer = std::make_shared<Renderer>(m_Spec.RenderSpec);
		m_Renderer->Init();

		LightManager::Init();

		m_ResourceManager = std::make_shared<ResourceManager>(m_Window->GetHandle(), m_Renderer->GetDevice(), m_Renderer->GetContext());
		m_Renderer->CreateShaderPrograms();
		m_Renderer->CreateInputLayouts();

		float fieldOfView = 3.141592654f / 4.f;
		float aspectRatio = (float)m_Spec.WinSpec.Width / (float)m_Spec.WinSpec.Height;
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, m_Spec.RenderSpec.NearPlane, m_Spec.RenderSpec.FarPlane);
		m_Camera = std::make_shared<Camera>(proj, m_Spec.RenderSpec.NearPlane, m_Spec.RenderSpec.FarPlane);
	}

	Application::~Application()
	{
		m_Layers.clear();
		
		m_Renderer->DestroyShaderPrograms();
		m_ResourceManager.reset();
		LightManager::Shutdown();
		m_Renderer.reset();
		InputHandler::Shutdown();
		m_Window.reset();
		
		ImGui::DestroyContext();

		CoUninitialize();
	}

	void Application::Run()
	{
		m_Running = true;

		m_AppStartTime = std::chrono::steady_clock::now();
		m_LastAppTime = m_AppStartTime;

		while (m_Running)
		{
			bool bShouldCenterCursor = m_NeedsCursorVisible.empty();
			InputHandler::HandleInputs(bShouldCenterCursor);

			if (m_Window->ShouldClose())
			{
				Stop();
				break;
			}

			if (GetForegroundWindow() != m_Window->GetHandle())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}

			UpdateAppTime();

			Core::VramInfo vramInfo = Core::Renderer::Get()->QueryVramUsage();
			double deltaTimeMs = GetAverageFrameTime();
			std::string frameTimeString = std::format("{:.1f}", deltaTimeMs);
			std::string vramUsage = std::to_string(vramInfo.CurrentUsage);
			std::string vramBudget = std::to_string(vramInfo.Budget);
			std::string newTitle = m_Spec.Name + " - Frame time: " + frameTimeString + "ms, VRAM: " + vramUsage + "/" + vramBudget + "MB";
			SetWindowText(m_Window->GetHandle(), newTitle.c_str());

			//Timer onUpdateTimer("OnUpdate");
			for (const std::unique_ptr<Layer>& layer : m_Layers)
				layer->OnUpdate(m_DeltaTime);
			//onUpdateTimer.EndTimer();

			m_Camera->CalcViewMatrix();
			LightManager::UpdateLightBufferData();
			Renderer::Get()->UpdateGlobalConstantBuffer(m_Camera.get(), (float)m_AppTime);

			Renderer::Get()->BeginScene();

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			//Timer onRenderTimer("OnRender");
			for (const std::unique_ptr<Layer>& layer : m_Layers)
				layer->OnRender(m_DeltaTime);
			//onRenderTimer.EndTimer();

			ImGui::EndFrame();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

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
		//std::println("{}", e.ToString());
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

		m_LastTenFrameTimes.insert(m_LastTenFrameTimes.begin(), m_DeltaTime * 1000.0);
		while (m_LastTenFrameTimes.size() > 10)
		{
			m_LastTenFrameTimes.pop_back();
		}
	}

	double Application::GetAverageFrameTime()
	{
		double sum = 0.0;
		for (double t : m_LastTenFrameTimes)
			sum += t;
		return sum / (double)m_LastTenFrameTimes.size();
	}

	void Application::UnregisterNeedForCursor(void* ptr)
	{
		m_NeedsCursorVisible.erase(ptr);
		if (m_NeedsCursorVisible.empty())
			InputHandler::CenterCursor();
	}
}