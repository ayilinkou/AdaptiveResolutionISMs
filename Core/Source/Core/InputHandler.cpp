#include "InputHandler.h"

namespace Core {
	InputHandler* InputHandler::s_pInstance = nullptr;
	std::function<void(Event&)> InputHandler::s_InputCallback = 0;
	double InputHandler::s_MouseDeltaX = 0.0;
	double InputHandler::s_MouseDeltaY = 0.0;
	POINT InputHandler::s_Center = {};
	HWND InputHandler::s_hwnd = 0;

	InputHandler* InputHandler::Init(std::function<void(Event&)> callback, HWND hwnd)
	{
		if (!s_pInstance)
			s_pInstance = new InputHandler();

		s_InputCallback = callback;
		s_hwnd = hwnd;

		RECT rect;
		GetClientRect(hwnd, &rect);
		s_Center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
		ClientToScreen(hwnd, &s_Center);
		SetCursorPos(s_Center.x, s_Center.y);

		return s_pInstance;
	}

	void InputHandler::Shutdown()
	{
		if (s_pInstance)
			delete s_pInstance;

		s_pInstance = nullptr;
	}

	void InputHandler::HandleInputs()
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));
		
		// handle the windows messages
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (GetForegroundWindow() == s_hwnd)
			SetCursorPos(s_Center.x, s_Center.y);
	}

	void InputHandler::ProcessMouseMovement()
	{
		POINT currentMousePos;
		GetCursorPos(&currentMousePos);
		s_MouseDeltaX = (double)(currentMousePos.x - s_Center.x);
		s_MouseDeltaY = (double)(s_Center.y - currentMousePos.y);
	}

	LRESULT InputHandler::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
	{
		switch (umsg)
		{
			case WM_KEYDOWN:
			{
				bool bIsRepeat = (lparam & (1 << 30)) != 0; // bit 30 signifies whether the input is a repeat or not
				KeyPressedEvent event(static_cast<int>(wparam), bIsRepeat);
				RaiseEvent(event);
				return 0;
			}
			case WM_KEYUP:
			{
				KeyReleasedEvent event(static_cast<int>(wparam));
				RaiseEvent(event);
				return 0;
			}
			case WM_MOUSEMOVE:
			{
				if (GetForegroundWindow() == s_hwnd)
				{
					ProcessMouseMovement();
					MouseMovedEvent event(s_MouseDeltaX, s_MouseDeltaY);
					RaiseEvent(event);
				}
				return 0;
			}
			default:
			{
				return DefWindowProc(hwnd, umsg, wparam, lparam);
			}
		}
	}

	void InputHandler::RaiseEvent(Event& e)
	{
		if (s_InputCallback)
			s_InputCallback(e);
	}
}