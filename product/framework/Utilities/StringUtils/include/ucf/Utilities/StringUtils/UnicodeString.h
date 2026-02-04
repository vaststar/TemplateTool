#pragma once

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <memory>
#include <iosfwd>

namespace ucf::utilities {

/**
 * @brief Unicode 字符串类
 * 
 * 内部使用 UTF-8 编码存储，提供与各种编码的无缝转换。
 * 
 * 设计目标：
 * - 跨平台：Windows/Linux/macOS 统一行为
 * - 类型安全：明确编码，避免隐式错误转换
 * - 高性能：内部 UTF-8，按需转换
 * - 易用性：兼容 std::string 接口
 * 
 * @code
 * // 构造
 * UnicodeString s1 = "hello";                          // ASCII
 * UnicodeString s2 = u8"你好";                         // UTF-8 字面量
 * UnicodeString s3 = UnicodeString::fromWide(L"你好"); // 从宽字符
 * UnicodeString s4 = UnicodeString::fromLocal(gbkStr); // 从本地编码
 * 
 * // 转换输出
 * std::cout << s1.toStdString();              // UTF-8 输出
 * SetWindowTextW(hwnd, s2.toWide().c_str());  // Windows API
 * 
 * // 字符操作
 * size_t len = s2.length();  // 字符数: 2
 * for (char32_t ch : s2) { } // 按码点遍历
 * @endcode
 */
class Utilities_EXPORT UnicodeString final
{
public:
    // ==========================================
    // 构造函数
    // ==========================================
    
    /// 默认构造（空字符串）
    UnicodeString();
    
    /// 从 UTF-8 std::string 构造
    UnicodeString(const std::string& utf8);
    
    /// 从 UTF-8 C 字符串构造
    UnicodeString(const char* utf8);
    
    /// 从 UTF-8 string_view 构造
    UnicodeString(std::string_view utf8);
    
    /// 从单个字符构造
    explicit UnicodeString(char32_t codepoint);

    ~UnicodeString();
    
    // 拷贝和移动
    UnicodeString(const UnicodeString& other);
    UnicodeString& operator=(const UnicodeString& other);
    UnicodeString(UnicodeString&& other) noexcept;
    UnicodeString& operator=(UnicodeString&& other) noexcept;

    // ==========================================
    // 静态工厂方法（显式编码转换）
    // ==========================================
    
    /// 从宽字符串创建（UTF-16 on Windows, UTF-32 on Linux）
    [[nodiscard]] static UnicodeString fromWide(const std::wstring& wide);
    [[nodiscard]] static UnicodeString fromWide(const wchar_t* wide);
    [[nodiscard]] static UnicodeString fromWide(const wchar_t* wide, size_t length);
    
    /// 从本地编码创建（Windows: GBK/ANSI, Linux: 通常是 UTF-8）
    [[nodiscard]] static UnicodeString fromLocal(const std::string& local);
    [[nodiscard]] static UnicodeString fromLocal(const char* local);
    
    /// 从 UTF-8 创建（与构造函数相同，但更明确）
    [[nodiscard]] static UnicodeString fromUtf8(const std::string& utf8);
    [[nodiscard]] static UnicodeString fromUtf8(const char* utf8);
    
    /// 从 UTF-16 创建
    [[nodiscard]] static UnicodeString fromUtf16(const std::u16string& utf16);
    [[nodiscard]] static UnicodeString fromUtf16(const char16_t* utf16);
    
    /// 从 UTF-32 创建
    [[nodiscard]] static UnicodeString fromUtf32(const std::u32string& utf32);
    [[nodiscard]] static UnicodeString fromUtf32(const char32_t* utf32);

    // ==========================================
    // 转换输出
    // ==========================================
    
    /// 转为 UTF-8 std::string
    [[nodiscard]] const std::string& toStdString() const noexcept;
    [[nodiscard]] std::string_view toStringView() const noexcept;
    
    /// 转为 C 风格字符串 (UTF-8)
    [[nodiscard]] const char* c_str() const noexcept;
    [[nodiscard]] const char* data() const noexcept;
    
    /// 转为宽字符串
    [[nodiscard]] std::wstring toWide() const;
    
    /// 转为本地编码
    [[nodiscard]] std::string toLocal() const;
    
    /// 转为 UTF-16
    [[nodiscard]] std::u16string toUtf16() const;
    
    /// 转为 UTF-32
    [[nodiscard]] std::u32string toUtf32() const;

    // ==========================================
    // 隐式转换
    // ==========================================
    
    /// 隐式转为 const std::string&
    operator const std::string&() const noexcept;
    
    /// 隐式转为 std::string_view
    operator std::string_view() const noexcept;

    // ==========================================
    // 长度和容量
    // ==========================================
    
    /// 字符长度（Unicode 码点数）
    [[nodiscard]] size_t length() const;
    [[nodiscard]] size_t size() const;
    
    /// 字节长度（UTF-8 字节数）
    [[nodiscard]] size_t byteLength() const noexcept;
    [[nodiscard]] size_t byteSize() const noexcept;
    
    /// 是否为空
    [[nodiscard]] bool empty() const noexcept;
    
    /// 清空
    void clear() noexcept;
    
    /// 预分配容量（字节）
    void reserve(size_t bytes);
    
    /// 当前容量
    [[nodiscard]] size_t capacity() const noexcept;

    // ==========================================
    // 字符访问（按码点索引）
    // ==========================================
    
    /// 获取指定位置的 Unicode 码点（带边界检查）
    [[nodiscard]] char32_t at(size_t index) const;
    
    /// 获取指定位置的 Unicode 码点（无边界检查）
    [[nodiscard]] char32_t operator[](size_t index) const;
    
    /// 获取第一个码点
    [[nodiscard]] char32_t front() const;
    
    /// 获取最后一个码点
    [[nodiscard]] char32_t back() const;

    // ==========================================
    // 迭代器（按码点迭代）
    // ==========================================
    
    class Utilities_EXPORT ConstIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = char32_t;
        using difference_type = std::ptrdiff_t;
        using pointer = const char32_t*;
        using reference = char32_t;

        ConstIterator() noexcept;
        
        reference operator*() const;
        ConstIterator& operator++();
        ConstIterator operator++(int);
        
        bool operator==(const ConstIterator& other) const noexcept;
        bool operator!=(const ConstIterator& other) const noexcept;

    private:
        friend class UnicodeString;
        const char* mPtr;
        const char* mEnd;
        
        ConstIterator(const char* ptr, const char* end) noexcept;
    };
    
    using const_iterator = ConstIterator;
    
    [[nodiscard]] ConstIterator begin() const noexcept;
    [[nodiscard]] ConstIterator end() const noexcept;
    [[nodiscard]] ConstIterator cbegin() const noexcept;
    [[nodiscard]] ConstIterator cend() const noexcept;

    // ==========================================
    // 运算符
    // ==========================================
    
    /// 连接
    [[nodiscard]] UnicodeString operator+(const UnicodeString& other) const;
    [[nodiscard]] UnicodeString operator+(const std::string& utf8) const;
    [[nodiscard]] UnicodeString operator+(const char* utf8) const;
    [[nodiscard]] UnicodeString operator+(char32_t codepoint) const;
    
    UnicodeString& operator+=(const UnicodeString& other);
    UnicodeString& operator+=(const std::string& utf8);
    UnicodeString& operator+=(const char* utf8);
    UnicodeString& operator+=(char32_t codepoint);
    
    /// 比较
    [[nodiscard]] bool operator==(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator!=(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator<(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator<=(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator>(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator>=(const UnicodeString& other) const noexcept;
    
    /// 与 std::string 比较
    [[nodiscard]] bool operator==(const std::string& other) const noexcept;
    [[nodiscard]] bool operator==(const char* other) const noexcept;

    // ==========================================
    // 验证
    // ==========================================
    
    /// 检查内部数据是否是有效的 UTF-8
    [[nodiscard]] bool isValid() const;
    
    /// 检查是否只包含 ASCII 字符
    [[nodiscard]] bool isAscii() const;
    
    /// 静态验证方法
    [[nodiscard]] static bool isValidUtf8(const std::string& str);
    [[nodiscard]] static bool isValidUtf8(const char* str, size_t length);

    // ==========================================
    // 子串（按码点索引）
    // ==========================================
    
    /// 获取子串
    [[nodiscard]] UnicodeString substr(size_t pos, size_t count = npos) const;

    // ==========================================
    // 常量
    // ==========================================
    
    static constexpr size_t npos = static_cast<size_t>(-1);

private:
    std::string mData;  // 内部 UTF-8 存储
    
    // 辅助方法
    [[nodiscard]] const char* pointerAt(size_t charIndex) const;
};

// ==========================================
// 非成员函数
// ==========================================

/// 流输出
Utilities_EXPORT std::ostream& operator<<(std::ostream& os, const UnicodeString& str);

/// 拼接运算符
[[nodiscard]] Utilities_EXPORT UnicodeString operator+(const char* lhs, const UnicodeString& rhs);
[[nodiscard]] Utilities_EXPORT UnicodeString operator+(const std::string& lhs, const UnicodeString& rhs);

/// 比较运算符
[[nodiscard]] Utilities_EXPORT bool operator==(const std::string& lhs, const UnicodeString& rhs) noexcept;
[[nodiscard]] Utilities_EXPORT bool operator==(const char* lhs, const UnicodeString& rhs) noexcept;

// ==========================================
// 字面量支持
// ==========================================

inline namespace literals {
    /// 字面量后缀: "hello"_us
    [[nodiscard]] inline UnicodeString operator""_us(const char* str, size_t len)
    {
        return UnicodeString(std::string_view(str, len));
    }
}

} // namespace ucf::utilities
