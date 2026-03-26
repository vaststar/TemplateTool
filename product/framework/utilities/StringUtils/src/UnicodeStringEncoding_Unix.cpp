#include "UnicodeStringEncoding.h"

#ifndef _WIN32

#include <cstdlib>
#include <cwchar>
#include <clocale>

namespace ucf::utilities::detail {

std::wstring utf8ToWide(const std::string& utf8)
{
    if (utf8.empty()) return {};
    
    // Linux/macOS: wchar_t 是 32 位 (UTF-32)
    std::u32string utf32 = utf8ToUtf32(utf8);
    
    std::wstring result;
    result.reserve(utf32.size());
    
    for (char32_t cp : utf32) {
        result.push_back(static_cast<wchar_t>(cp));
    }
    
    return result;
}

std::string wideToUtf8(const std::wstring& wide)
{
    return wideToUtf8(wide.data(), wide.size());
}

std::string wideToUtf8(const wchar_t* wide, size_t length)
{
    if (length == 0 || wide == nullptr) return {};
    
    // Linux/macOS: wchar_t 是 32 位 (UTF-32)
    std::u32string utf32;
    utf32.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        utf32.push_back(static_cast<char32_t>(wide[i]));
    }
    
    return utf32ToUtf8(utf32);
}

std::string localToUtf8(const std::string& local)
{
    // Linux/macOS 通常已经是 UTF-8
    // 如果需要支持其他编码，可以使用 iconv
    return local;
}

std::string localToUtf8(const char* local, size_t length)
{
    if (local == nullptr) return {};
    return std::string(local, length);
}

std::string utf8ToLocal(const std::string& utf8)
{
    // Linux/macOS 通常已经是 UTF-8
    return utf8;
}

} // namespace ucf::utilities::detail

#endif // !_WIN32
