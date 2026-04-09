#pragma once

#include "d3d11.h"

namespace Core {
	class Logger
	{
	public:
		static void Init(HWND hwnd) { s_hwnd = hwnd; }
	
		static void OutputShaderErrorMessage(ID3D10Blob* errorMessage, const char* shaderFilename);
		static void ShowMessageBox(const char* message, const char* title, UINT flags = MB_OK);

	private:
		static HWND s_hwnd;
	};
}
