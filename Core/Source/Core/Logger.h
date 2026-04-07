#pragma once

#include "d3d11.h"

class Logger
{
public:
	static void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hWnd, const char* shaderFilename);
};
