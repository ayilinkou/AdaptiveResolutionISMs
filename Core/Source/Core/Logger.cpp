#include <cstring>
#include <fstream>

#include "Logger.h"

void Logger::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hWnd, const char* shaderFilename)
{
	unsigned long long bufferSize, i;
	std::ofstream fout;

	std::string str1 = "Error compiling shader!\n\n";
	std::string compileErrors = (char*)(errorMessage->GetBufferPointer());
	bufferSize = errorMessage->GetBufferSize();

	fout.open("shader-error.txt");
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors.c_str()[i];
	}

	fout.close();

	errorMessage->Release();
	errorMessage = 0;

	MessageBox(hWnd, (str1 + compileErrors).c_str(), shaderFilename, MB_OK);
}
