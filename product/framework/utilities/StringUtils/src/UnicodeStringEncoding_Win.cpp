#include "UnicodeStringEncoding.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ucf::utilities::detail {

std::wstring utf8ToWide(const std::string& utf8)
{
    if (utf8.empty()) return {};
    
    int size = MultiByteToWideChar(
        CP_UTF8, 0,
        utf8.data(), static_cast<int>(utf8.size()),
        nullptr, 0);
    
    if (size <= 0) return {};
    
    std::wstring result(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(
        CP_UTF8, 0,
        utf8.data(), static_cast<int>(utf8.size()),
        result.data(), size);
    
    return result;
}

std::string wideToUtf8(const std::wstring& wide)
{
    return wideToUtf8(wide.data(), wide.size());
}

std::string wideToUtf8(const wchar_t* wide, size_t length)
{
    if (length == 0 || wide == nullptr) return {};
    
    int size = WideCharToMultiByte(
        CP_UTF8, 0,
        wide, static_cast<int>(length),
        nullptr, 0, nullptr, nullptr);
    
    if (size <= 0) return {};
    
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(
        CP_UTF8, 0,
        wide, static_cast<int>(length),
        result.data(), size, nullptr, nullptr);
    
    return result;
}

std::string localToUtf8(const std::string& local)
{
    return localToUtf8(local.data(), local.size());
}

std::string localToUtf8(const char* local, size_t length)
{
    if (length == 0 || local == nullptr) return {};
    
    // Step 1: Local (ANSI) -> Wide (UTF-16)
    int wideSize = MultiByteToWideChar(
        CP_ACP, 0,
        local, static_cast<int>(length),
        nullptr, 0);
    
    if (wideSize <= 0) return std::string(local, length);  // fallback
    
    std::wstring wide(static_cast<size_t>(wideSize), L'\0');
    MultiByteToWideChar(
        CP_ACP, 0,
        local, static_cast<int>(length),
        wide.data(), wideSize);
    
    // Step 2: Wide (UTF-16) -> UTF-8
    return wideToUtf8(wide);
}

std::string utf8ToLocal(const std::string& utf8)
{
    if (utf8.empty()) return {};
    
    // Step 1: UTF-8 -> Wide (UTF-16)
    std::wstring wide = utf8ToWide(utf8);
    if (wide.empty()) return utf8;  // fallback
    
    // Step 2: Wide (UTF-16) -> Local (ANSI)
    int localSize = WideCharToMultiByte(
        CP_ACP, 0,
        wide.data(), static_cast<int>(wide.size()),
        nullptr, 0, nullptr, nullptr);
    
    if (localSize <= 0) return utf8;  // fallback
    
    std::string result(static_cast<size_t>(localSize), '\0');
    WideCharToMultiByte(
        CP_ACP, 0,
        wide.data(), static_cast<int>(wide.size()),
        result.data(), localSize, nullptr, nullptr);
    
    return result;
}

} // namespace ucf::utilities::detail

#endif // _WIN32
