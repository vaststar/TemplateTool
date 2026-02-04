#include "UnicodeStringEncoding.h"

namespace ucf::utilities::detail {

char32_t decodeUtf8(const char*& ptr, const char* end)
{
    if (ptr >= end) return 0xFFFD;
    
    unsigned char c = static_cast<unsigned char>(*ptr);
    char32_t codepoint;
    int remaining;
    
    if ((c & 0x80) == 0) {
        // ASCII: 0xxxxxxx
        codepoint = c;
        remaining = 0;
    } else if ((c & 0xE0) == 0xC0) {
        // 2-byte: 110xxxxx
        codepoint = c & 0x1F;
        remaining = 1;
    } else if ((c & 0xF0) == 0xE0) {
        // 3-byte: 1110xxxx
        codepoint = c & 0x0F;
        remaining = 2;
    } else if ((c & 0xF8) == 0xF0) {
        // 4-byte: 11110xxx
        codepoint = c & 0x07;
        remaining = 3;
    } else {
        // Invalid leading byte
        ++ptr;
        return 0xFFFD;
    }
    
    ++ptr;
    
    for (int i = 0; i < remaining; ++i) {
        if (ptr >= end) return 0xFFFD;
        c = static_cast<unsigned char>(*ptr);
        if ((c & 0xC0) != 0x80) return 0xFFFD;  // Invalid continuation
        codepoint = (codepoint << 6) | (c & 0x3F);
        ++ptr;
    }
    
    return codepoint;
}

void encodeUtf8(char32_t codepoint, std::string& out)
{
    if (codepoint <= 0x7F) {
        out.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0x10FFFF) {
        out.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        // Invalid codepoint, use replacement character
        encodeUtf8(0xFFFD, out);
    }
}

size_t countCodepoints(const std::string& utf8)
{
    size_t count = 0;
    const char* ptr = utf8.data();
    const char* end = ptr + utf8.size();
    
    while (ptr < end) {
        decodeUtf8(ptr, end);
        ++count;
    }
    return count;
}

const char* codepointPointer(const std::string& utf8, size_t index)
{
    const char* ptr = utf8.data();
    const char* end = ptr + utf8.size();
    
    for (size_t i = 0; i < index && ptr < end; ++i) {
        decodeUtf8(ptr, end);
    }
    return ptr;
}

bool validateUtf8(const char* data, size_t length)
{
    const unsigned char* ptr = reinterpret_cast<const unsigned char*>(data);
    const unsigned char* end = ptr + length;
    
    while (ptr < end) {
        if (*ptr <= 0x7F) {
            ++ptr;
        } else if ((*ptr & 0xE0) == 0xC0) {
            if (ptr + 1 >= end || (ptr[1] & 0xC0) != 0x80) return false;
            // Check for overlong encoding
            if ((*ptr & 0x1E) == 0) return false;
            ptr += 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            if (ptr + 2 >= end || (ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80) return false;
            // Check for overlong encoding and surrogate range
            if (*ptr == 0xE0 && (ptr[1] & 0x20) == 0) return false;
            ptr += 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            if (ptr + 3 >= end || (ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80 || (ptr[3] & 0xC0) != 0x80) return false;
            // Check for overlong encoding and max codepoint
            if (*ptr == 0xF0 && (ptr[1] & 0x30) == 0) return false;
            if (*ptr == 0xF4 && ptr[1] > 0x8F) return false;
            if (*ptr > 0xF4) return false;
            ptr += 4;
        } else {
            return false;
        }
    }
    return true;
}

bool checkAscii(const char* data, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        if (static_cast<unsigned char>(data[i]) > 0x7F) {
            return false;
        }
    }
    return true;
}

// UTF-16 <-> UTF-8 (跨平台)
std::string utf16ToUtf8(const std::u16string& utf16)
{
    return utf16ToUtf8(utf16.data(), utf16.size());
}

std::string utf16ToUtf8(const char16_t* utf16, size_t length)
{
    std::string result;
    result.reserve(length * 3);  // 预估
    
    for (size_t i = 0; i < length; ++i) {
        char32_t codepoint = utf16[i];
        
        // Handle surrogate pairs
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF && i + 1 < length) {
            char16_t low = utf16[i + 1];
            if (low >= 0xDC00 && low <= 0xDFFF) {
                codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                ++i;
            }
        }
        
        encodeUtf8(codepoint, result);
    }
    return result;
}

std::u16string utf8ToUtf16(const std::string& utf8)
{
    std::u16string result;
    const char* ptr = utf8.data();
    const char* end = ptr + utf8.size();
    
    while (ptr < end) {
        char32_t cp = decodeUtf8(ptr, end);
        
        if (cp <= 0xFFFF) {
            result.push_back(static_cast<char16_t>(cp));
        } else {
            // Encode as surrogate pair
            cp -= 0x10000;
            result.push_back(static_cast<char16_t>(0xD800 + (cp >> 10)));
            result.push_back(static_cast<char16_t>(0xDC00 + (cp & 0x3FF)));
        }
    }
    return result;
}

// UTF-32 <-> UTF-8 (跨平台)
std::string utf32ToUtf8(const std::u32string& utf32)
{
    return utf32ToUtf8(utf32.data(), utf32.size());
}

std::string utf32ToUtf8(const char32_t* utf32, size_t length)
{
    std::string result;
    result.reserve(length * 4);
    
    for (size_t i = 0; i < length; ++i) {
        encodeUtf8(utf32[i], result);
    }
    return result;
}

std::u32string utf8ToUtf32(const std::string& utf8)
{
    std::u32string result;
    const char* ptr = utf8.data();
    const char* end = ptr + utf8.size();
    
    while (ptr < end) {
        result.push_back(decodeUtf8(ptr, end));
    }
    return result;
}

} // namespace ucf::utilities::detail
