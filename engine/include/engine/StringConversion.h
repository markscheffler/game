#pragma once
#include <string>

namespace fire::ConvertString
{
        std::string narrow(const std::wstring& in);
        std::string narrow(const wchar_t* in);
        std::wstring widen(const std::string& in);
        std::wstring widen(const char* in);
}