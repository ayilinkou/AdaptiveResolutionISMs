#include <fstream>

#include "Logger.h"

namespace Core {
	HWND Logger::s_hwnd = 0;

	void Logger::OutputShaderErrorMessage(ID3D10Blob* errorMessage, const char* shaderFilename)
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

		MessageBox(s_hwnd, (str1 + compileErrors).c_str(), shaderFilename, MB_OK | MB_ICONERROR);
	}
	void Logger::ShowMessageBox(const char* message, const char* title, UINT flags)
	{
		MessageBox(s_hwnd, message, title, flags);
	}
}
