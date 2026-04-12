#pragma once

#include <string>
#include <vector>
#include <stringapiset.h>
#include <shobjidl.h>

namespace Core {
	struct StaticUtils
	{
		static void ToWideString(const char* s, std::wstring& ws)
		{
			int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
			if (sizeNeeded <= 0)
			{
				ws.clear();
				return;
			}

			std::vector<wchar_t> buffer(sizeNeeded);
			MultiByteToWideChar(CP_UTF8, 0, s, -1, buffer.data(), sizeNeeded);
			ws = std::wstring(buffer.data());
		}

		static void ToNarrowString(const std::wstring& ws, std::string& str)
		{
			if (ws.empty())
			{
				str = "";
				return;
			}

			int sizeNeeded = WideCharToMultiByte(
				CP_UTF8, 0,
				ws.data(), (int)ws.size(),
				nullptr, 0,
				nullptr, nullptr
			);

			str.resize(sizeNeeded, 0);

			WideCharToMultiByte(
				CP_UTF8, 0,
				ws.data(), (int)ws.size(),
				str.data(), sizeNeeded,
				nullptr, nullptr
			);
		}

		static std::wstring OpenFileDialog()
		{
			std::vector<COMDLG_FILTERSPEC> filters;
			return OpenFileDialog(filters);
		}

		static std::wstring OpenFileDialog(std::vector<COMDLG_FILTERSPEC> filters)
		{
			HRESULT hResult;
			std::wstring path;

			IFileOpenDialog* pFileOpen = nullptr;

			hResult = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

			if (SUCCEEDED(hResult))
			{
				if (filters.empty())
				{
					filters.push_back({ L"All Files", L"*.*" });
				}

				pFileOpen->SetFileTypes((UINT)filters.size(), filters.data());
				pFileOpen->SetFileTypeIndex(1);
				hResult = pFileOpen->Show(nullptr);

				if (SUCCEEDED(hResult))
				{
					IShellItem* pItem = nullptr;
					hResult = pFileOpen->GetResult(&pItem);

					if (SUCCEEDED(hResult))
					{
						PWSTR filePath = nullptr;
						hResult = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);

						if (SUCCEEDED(hResult))
						{
							path = filePath;
							CoTaskMemFree(filePath);
						}
						pItem->Release();
					}
				}
				pFileOpen->Release();
			}

			return path;
		}

		static std::string_view GetFileExtension(const char* s)
		{
			if (!s)
				return {};

			const char* dot = std::strchr(s, '.');
			if (!dot || *(dot + 1) == '\0')
				return {};

			return std::string_view(dot + 1);
		}

		static bool IsCursorVisible()
		{
			CURSORINFO ci{};
			ci.cbSize = sizeof(ci);

			if (GetCursorInfo(&ci))
				return (ci.flags & CURSOR_SHOWING) != 0;
			return false;
		}
	};
}