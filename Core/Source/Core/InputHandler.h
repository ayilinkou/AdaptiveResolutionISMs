#pragma once

#define WIN32_LEAN_AND_MEAN

#include <functional>

#include "Windows.h"

#include "InputEvents.h"

namespace Core {
	class InputHandler
	{
	private:
		InputHandler();

	public:
		static InputHandler* Init(std::function<void(Event&)> callback);
		static void Shutdown();
		
		static void HandleInputs();
		static InputHandler* Get() { return s_pInstance; }

		static LRESULT MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	private:
		static void RaiseEvent(Event& e);

	private:
		static InputHandler* s_pInstance;
		static std::function<void(Event&)> s_InputCallback;
	};
}