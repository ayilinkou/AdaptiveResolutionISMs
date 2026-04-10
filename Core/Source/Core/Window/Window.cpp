#include "imgui_impl_win32.h"

#include "Window.h"
#include "Core/Input/InputHandler.h"
#include "Core/Application/Application.h"

namespace Core {
	Window::Window(const WindowSpec& spec)
		: m_Spec(spec)
	{
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Init()
	{
		WNDCLASSEX wc;
		DEVMODE dmScreenSettings;
		int posX, posY;

		m_hInstance = GetModuleHandle(NULL);

		// setup the windows class with default settings
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = m_hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = m_Spec.Title.c_str();
		wc.cbSize = sizeof(WNDCLASSEX);

		// register the window class
		RegisterClassEx(&wc);

		unsigned long screenWidth = GetSystemMetrics(SM_CXSCREEN);
		unsigned long screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// setup the screen settings depending on whether it is running in full screen or in windowed mode
		if (m_Spec.bFullscreen)
		{
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = screenWidth;
			dmScreenSettings.dmPelsHeight = screenHeight;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

			posX = posY = 0;
		}
		else
		{
			screenWidth = m_Spec.Width;
			screenHeight = m_Spec.Height;

			posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
			posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
		}

		// only include top bar if not in fullscreen
		DWORD windowStyle = m_Spec.bFullscreen ? WS_POPUP : (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);

		// get window dimensions from client size + styling
		RECT desiredRect = { 0, 0, (LONG)screenWidth, (LONG)screenHeight };
		AdjustWindowRectEx(&desiredRect, windowStyle, FALSE, WS_EX_APPWINDOW);

		int windowWidth = desiredRect.right - desiredRect.left;
		int windowHeight = desiredRect.bottom - desiredRect.top;

		m_hwnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			m_Spec.Title.c_str(),
			m_Spec.Title.c_str(),
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | windowStyle,
			posX, posY,
			windowWidth, windowHeight,
			NULL,
			NULL,
			m_hInstance,
			NULL
		);

		ShowWindow(m_hwnd, SW_SHOW);
		SetForegroundWindow(m_hwnd);
		SetFocus(m_hwnd);

		ShowCursor(false);

		ImGui_ImplWin32_Init(m_hwnd);

		RECT rect;
		GetClientRect(m_hwnd, &rect);
		POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
		ClientToScreen(m_hwnd, &center);
		SetCursorPos(center.x, center.y);
	}

	void Window::Shutdown()
	{
		ImGui_ImplWin32_Shutdown();
		
		ShowCursor(true);
		ClipCursor(NULL);

		if (m_Spec.bFullscreen)
		{
			ChangeDisplaySettings(NULL, 0);
		}

		DestroyWindow(m_hwnd);
		m_hwnd = NULL;

		UnregisterClass(m_Spec.Title.c_str(), m_hInstance);
		m_hInstance = NULL;
	}

	bool Window::ShouldClose() const
	{
		return bShouldClose;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_CLOSE:
		{
			Core::Application::Get()->Stop();
			return 0;
		}
		default:
		{
			return Core::InputHandler::MessageHandler(hWnd, uMessage, wParam, lParam);
		}
	}
}