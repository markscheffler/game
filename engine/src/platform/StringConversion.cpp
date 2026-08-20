#include <StringConversion.h>
#include <Windows.h>


namespace fire::ConvertString
{
    std::string narrow(const std::wstring& in)
    {
        if(in.empty()) return std::string();
        auto size_needed = WideCharToMultiByte(CP_UTF8,
        0,
        in.data(),
        static_cast<int>(in.size()),
        nullptr,
        0,
        nullptr,
        nullptr);

        std::string out(size_needed, '\0');
        WideCharToMultiByte(CP_UTF8,
        0,
        in.data(),
        static_cast<int>(in.size()),
        out.data(),
        size_needed,
        nullptr,
        nullptr);

        return out;
    }

    std::string narrow(const wchar_t* in)
    {
        return narrow(std::wstring(in));
    }

    std::wstring widen(const std::string& in)
    {
        if(in.empty()) return std::wstring(); 
        auto size_needed = MultiByteToWideChar(CP_UTF8,
        0,
        in.data(),
        static_cast<int>(in.size()),
        nullptr,
        0);

        std::wstring out(size_needed, L'\0');
        MultiByteToWideChar(CP_UTF8,
        0,
        in.data(),
        static_cast<int>(in.size()),
        out.data(),
        size_needed);
        
        return out;
    }

    std::wstring widen(const char* in)
    {
        return widen(std::string(in));
    }
}