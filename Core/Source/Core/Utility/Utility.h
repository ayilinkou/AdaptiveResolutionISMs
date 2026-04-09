#pragma once

#include <string>
#include <vector>
#include <stringapiset.h>

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

    static std::string_view GetFileExtension(const char* s)
    {
        if (!s)
            return {};

        const char* dot = std::strchr(s, '.');
        if (!dot || *(dot + 1) == '\0')
            return {};

        return std::string_view(dot + 1);
    }
};