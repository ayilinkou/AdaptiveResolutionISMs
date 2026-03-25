#include "Window.h"
#include "InputHandler.h"
#include "Application.h"

namespace Core {
	Window::Window(const WindowSpec& spec)
		: m_Spec(spec)
	{
	}

	Window::~Window()
	{
		Destroy();
	}

	void Window::Create()
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

		unsigned long ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		unsigned long ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

		// setup the screen settings depending on whether it is running in full screen or in windowed mode
		if (m_Spec.bFullscreen)
		{
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = ScreenWidth;
			dmScreenSettings.dmPelsHeight = ScreenHeight;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

			posX = posY = 0;
		}
		else
		{
			ScreenWidth = m_Spec.Width;
			ScreenHeight = m_Spec.Height;

			posX = (GetSystemMetrics(SM_CXSCREEN) - ScreenWidth) / 2;
			posY = (GetSystemMetrics(SM_CYSCREEN) - ScreenHeight) / 2;
		}

		// only include top bar if not in fullscreen
		DWORD windowTopBar = m_Spec.bFullscreen ? WS_POPUP : (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
		m_hwnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			m_Spec.Title.c_str(),
			m_Spec.Title.c_str(),
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | windowTopBar,
			posX, posY,
			ScreenWidth, ScreenHeight,
			NULL,
			NULL,
			m_hInstance,
			NULL
		);

		ShowWindow(m_hwnd, SW_SHOW);
		SetForegroundWindow(m_hwnd);
		SetFocus(m_hwnd);

		ShowCursor(true);

		RECT rect;
		GetClientRect(m_hwnd, &rect);
		POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
		ClientToScreen(m_hwnd, &center);
		SetCursorPos(center.x, center.y);
	}

	void Window::Destroy()
	{
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

	void Window::Update()
	{
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