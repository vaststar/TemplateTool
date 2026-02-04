#pragma once

#include <string>
#include <cstdint>

namespace ucf::utilities::detail {

// ==========================================
// UTF-8 编解码
// ==========================================

/// 从 UTF-8 序列解码一个码点，更新指针位置
/// @return 解码的码点，失败返回 0xFFFD (replacement character)
char32_t decodeUtf8(const char*& ptr, const char* end);

/// 将码点编码为 UTF-8 追加到字符串
void encodeUtf8(char32_t codepoint, std::string& out);

/// 计算 UTF-8 字符串的码点数量
size_t countCodepoints(const std::string& utf8);

/// 获取指向第 n 个码点的指针
const char* codepointPointer(const std::string& utf8, size_t index);

// ==========================================
// 编码验证
// ==========================================

/// 验证 UTF-8 字符串
bool validateUtf8(const char* data, size_t length);

/// 检查是否全是 ASCII
bool checkAscii(const char* data, size_t length);

// ==========================================
// 编码转换（平台相关实现）
// ==========================================

/// UTF-8 -> 宽字符 (wchar_t)
std::wstring utf8ToWide(const std::string& utf8);

/// 宽字符 -> UTF-8
std::string wideToUtf8(const std::wstring& wide);
std::string wideToUtf8(const wchar_t* wide, size_t length);

/// 本地编码 -> UTF-8
std::string localToUtf8(const std::string& local);
std::string localToUtf8(const char* local, size_t length);

/// UTF-8 -> 本地编码
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
