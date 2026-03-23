#include "InputHandler.h"

namespace Core {
	InputHandler* InputHandler::s_pInstance = nullptr;
	std::function<void(Event&)> InputHandler::s_InputCallback = 0;

	InputHandler::InputHandler()
	{

	}

	InputHandler* InputHandler::Init(std::function<void(Event&)> callback)
	{
		if (!s_pInstance)
			s_pInstance = new InputHandler();

		s_InputCallback = callback;
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