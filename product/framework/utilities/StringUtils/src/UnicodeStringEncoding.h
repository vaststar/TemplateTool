#pragma once

#include <string>
#include <cstdint>

namespace ucf::utilities::detail {

// ==========================================
// UTF-8 Encoding/Decoding
// ==========================================

/// Decode one code point from UTF-8 sequence, updates pointer position
/// @return The decoded code point, or 0xFFFD (replacement character) on failure
char32_t decodeUtf8(const char*& ptr, const char* end);

/// Encode code point as UTF-8 and append to string
void encodeUtf8(char32_t codepoint, std::string& out);

/// Count the number of code points in a UTF-8 string
size_t countCodepoints(const std::string& utf8);

/// Get pointer to the n-th code point
const char* codepointPointer(const std::string& utf8, size_t index);

// ==========================================
// Encoding Validation
// ==========================================

/// Validate UTF-8 string
bool validateUtf8(const char* data, size_t length);

/// Check if all characters are ASCII
bool checkAscii(const char* data, size_t length);

// ==========================================
// Encoding Conversion (platform-specific implementation)
// ==========================================

/// UTF-8 -> wide string (wchar_t)
std::wstring utf8ToWide(const std::string& utf8);

/// Wide string -> UTF-8
std::string wideToUtf8(const std::wstring& wide);
std::string wideToUtf8(const wchar_t* wide, size_t length);

/// Local encoding -> UTF-8
std::string localToUtf8(const std::string& local);
std::string localToUtf8(const char* local, size_t length);

/// UTF-8 -> local encoding
std::string utf8ToLocal(const std::string& utf8);

/// UTF-16 -> UTF-8
std::string utf16ToUtf8(const std::u16string& utf16);
std::string utf16ToUtf8(const char16_t* utf16, size_t length);

/// UTF-8 -> UTF-16
std::u16string utf8ToUtf16(const std::string& utf8);

/// UTF-32 -> UTF-8
std::string utf32ToUtf8(const std::u32string& utf32);
std::string utf32ToUtf8(const char32_t* utf32, size_t length);

/// UTF-8 -> UTF-32
std::u32string utf8ToUtf32(const std::string& utf8);

} // namespace ucf::utilities::detail
