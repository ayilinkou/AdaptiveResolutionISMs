#pragma once

#define WIN32_LEAN_AND_MEAN

#include <string>

#include <Windows.h>

namespace Core {
	struct WindowSpec
	{
		std::string Title = "Window";
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool bUseVSync = false;
		bool bFullscreen = false;
	};

	class Window
	{
	public:
		Window() = delete;
		Window(const WindowSpec& spec = WindowSpec());
		~Window();

		void Create();
		void Destroy();

		void Update();

		bool ShouldClose() const;

		HWND GetHandle() const { return m_hwnd; }

	private:
		bool bShouldClose = false;
		
		WindowSpec m_Spec;

		HWND m_hwnd;
		HINSTANCE m_hInstance;
	};
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
