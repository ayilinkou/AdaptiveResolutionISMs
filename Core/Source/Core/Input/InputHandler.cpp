#include "imgui_impl_win32.h"

#include "InputHandler.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

	void InputHandler::HandleInputs(bool bCenterCursor)
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		ProcessKeyboardMovement();
		
		// handle the windows messages
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (GetForegroundWindow() == s_hwnd && bCenterCursor)
			SetCursorPos(s_Center.x, s_Center.y);
	}

	void InputHandler::ProcessMouseMovement()
	{
		POINT currentMousePos;
		GetCursorPos(&currentMousePos);
		s_MouseDeltaX = (double)(currentMousePos.x - s_Center.x);
		s_MouseDeltaY = (double)(s_Center.y - currentMousePos.y);
	}

	void InputHandler::ProcessKeyboardMovement()
	{
		/*
			TODO: These keys are important, we can't afford to have the keyboard delay and so we check them every frame.
			This might have an impact on performance if we do this with too many keys. Might be worth to make a system
			which only runs this on keys which have been detected to be pressed down by WM_KEYDOWN, and then stop polling after WM_KEYUP.
		*/
		
		// bit 0x8000 signals if the key is currently down
		if (GetAsyncKeyState('W') & 0x8000)
		{
			KeyPressedEvent event(static_cast<int>('W'), true);
			RaiseEvent(event);
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			KeyPressedEvent event(static_cast<int>('S'), true);
			RaiseEvent(event);
		}
		if (GetAsyncKeyState('A') & 0x8000)
		{
			KeyPressedEvent event(static_cast<int>('A'), true);
			RaiseEvent(event);
		}
		if (GetAsyncKeyState('D') & 0x8000)
		{
			KeyPressedEvent event(static_cast<int>('D'), true);
			RaiseEvent(event);
		}
		if (GetAsyncKeyState('Q') & 0x8000)
		{
			KeyPressedEvent event(static_cast<int>('Q'), true);
			RaiseEvent(event);
		}
		if (GetAsyncKeyState('E') & 0x8000)
		{
			KeyPressedEvent event(static_cast<int>('E'), true);
			RaiseEvent(event);
		}
	}

	LRESULT InputHandler::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, umsg, wparam, lparam))
		{
			return true;
		}
		
		switch (umsg)
		{
			// These key presses include the delay before repeating. For smooth keyboard inputs, use GetAsyncKeyState() instead.
			case WM_KEYDOWN:
			{
				if (IsMovementKey(wparam)) // these events were already raised in ProcessKeyboardMovement()
					return 0;

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
					if (s_MouseDeltaX != 0.0 && s_MouseDeltaY != 0.0)
					{
						MouseMovedEvent event(s_MouseDeltaX, s_MouseDeltaY);
						RaiseEvent(event);
					}
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

	bool InputHandler::IsMovementKey(WPARAM wparam)
	{
		switch (wparam)
		{
		case 'W':
		case 'A':
		case 'S':
		case 'D':
		case 'Q':
		case 'E':
			return true;
		default:
			return false;
		}
	}
}