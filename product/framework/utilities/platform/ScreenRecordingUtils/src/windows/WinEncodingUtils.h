#pragma once

#ifdef _WIN32

#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ucf::utilities::screenrecording {

inline std::string wideToUtf8(const wchar_t* wstr)
{
    if (!wstr) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), len, nullptr, nullptr);
    return result;
}

inline std::vector<wchar_t> utf8ToWide(const std::string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> wide(len);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide.data(), len);
    return wide;
}

} // namespace ucf::utilities::screenrecording

#endif // _WIN32
