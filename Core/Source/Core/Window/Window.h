#pragma once

#define WIN32_LEAN_AND_MEAN

#include <string>

#include <Windows.h>

namespace Core {
	enum class WindowType
	{
		Windowed,
		Borderless,
		Fullscreen
	};
	
	struct WindowSpec
	{
		std::string Title = "Window";
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool bUseVSync = false;
		WindowType Type = WindowType::Windowed;
	};

	class Window
	{
	public:
		Window() = delete;
		Window(const WindowSpec& spec = WindowSpec());
		~Window();

		void Init();
		void Shutdown();

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
