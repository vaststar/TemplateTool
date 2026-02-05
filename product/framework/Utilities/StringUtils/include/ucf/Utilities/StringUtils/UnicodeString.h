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
 * @brief Unicode string class
 * 
 * Uses UTF-8 encoding internally, providing seamless conversion to various encodings.
 * 
 * Design goals:
 * - Cross-platform: Consistent behavior across Windows/Linux/macOS
 * - Type-safe: Explicit encoding, avoiding implicit incorrect conversions
 * - High performance: Internal UTF-8, convert on demand
 * - Ease of use: Compatible with std::string interface
 * 
 * @code
 * // Construction
 * UnicodeString s1 = "hello";                          // ASCII
 * UnicodeString s2 = u8"你好";                         // UTF-8 literal
 * UnicodeString s3 = UnicodeString::fromWide(L"你好"); // From wide string
 * UnicodeString s4 = UnicodeString::fromLocal(gbkStr); // From local encoding
 * 
 * // Conversion output
 * std::cout << s1.toStdString();              // UTF-8 output
 * SetWindowTextW(hwnd, s2.toWide().c_str());  // Windows API
 * 
 * // Character operations
 * size_t len = s2.length();  // Character count: 2
 * for (char32_t ch : s2) { } // Iterate by code point
 * @endcode
 */
class Utilities_EXPORT UnicodeString final
{
public:
    // ==========================================
    // Constructors
    // ==========================================
    
    /// Default constructor (empty string)
    UnicodeString();
    
    /// Construct from UTF-8 std::string
    UnicodeString(const std::string& utf8);
    
    /// Construct from UTF-8 C string
    UnicodeString(const char* utf8);
    
    /// Construct from UTF-8 string_view
    UnicodeString(std::string_view utf8);
    
    /// Construct from single character
    explicit UnicodeString(char32_t codepoint);

    ~UnicodeString();
    
    // Copy and move
    UnicodeString(const UnicodeString& other);
    UnicodeString& operator=(const UnicodeString& other);
    UnicodeString(UnicodeString&& other) noexcept;
    UnicodeString& operator=(UnicodeString&& other) noexcept;

    // ==========================================
    // Static Factory Methods (explicit encoding conversion)
    // ==========================================
    
    /// Create from wide string (UTF-16 on Windows, UTF-32 on Linux)
    [[nodiscard]] static UnicodeString fromWide(const std::wstring& wide);
    [[nodiscard]] static UnicodeString fromWide(const wchar_t* wide);
    [[nodiscard]] static UnicodeString fromWide(const wchar_t* wide, size_t length);
    
    /// Create from local encoding (Windows: GBK/ANSI, Linux: usually UTF-8)
    [[nodiscard]] static UnicodeString fromLocal(const std::string& local);
    [[nodiscard]] static UnicodeString fromLocal(const char* local);
    
    /// Create from UTF-8 (same as constructor, but more explicit)
    [[nodiscard]] static UnicodeString fromUtf8(const std::string& utf8);
    [[nodiscard]] static UnicodeString fromUtf8(const char* utf8);
    
    /// Create from UTF-16
    [[nodiscard]] static UnicodeString fromUtf16(const std::u16string& utf16);
    [[nodiscard]] static UnicodeString fromUtf16(const char16_t* utf16);
    
    /// Create from UTF-32
    [[nodiscard]] static UnicodeString fromUtf32(const std::u32string& utf32);
    [[nodiscard]] static UnicodeString fromUtf32(const char32_t* utf32);

    // ==========================================
    // Conversion Output
    // ==========================================
    
    /// Convert to UTF-8 std::string
    [[nodiscard]] const std::string& toStdString() const noexcept;
    [[nodiscard]] std::string_view toStringView() const noexcept;
    
    /// Convert to C-style string (UTF-8)
    [[nodiscard]] const char* c_str() const noexcept;
    [[nodiscard]] const char* data() const noexcept;
    
    /// Convert to wide string
    [[nodiscard]] std::wstring toWide() const;
    
    /// Convert to local encoding
    [[nodiscard]] std::string toLocal() const;
    
    /// Convert to UTF-16
    [[nodiscard]] std::u16string toUtf16() const;
    
    /// Convert to UTF-32
    [[nodiscard]] std::u32string toUtf32() const;

    // ==========================================
    // Implicit Conversion
    // ==========================================
    
    /// Implicitly convert to const std::string&
    operator const std::string&() const noexcept;
    
    /// Implicitly convert to std::string_view
    operator std::string_view() const noexcept;

    // ==========================================
    // Length and Capacity
    // ==========================================
    
    /// Character length (Unicode code point count)
    [[nodiscard]] size_t length() const;
    [[nodiscard]] size_t size() const;
    
    /// Byte length (UTF-8 byte count)
    [[nodiscard]] size_t byteLength() const noexcept;
    [[nodiscard]] size_t byteSize() const noexcept;
    
    /// Check if empty
    [[nodiscard]] bool empty() const noexcept;
    
    /// Clear
    void clear() noexcept;
    
    /// Reserve capacity (bytes)
    void reserve(size_t bytes);
    
    /// Current capacity
    [[nodiscard]] size_t capacity() const noexcept;

    // ==========================================
    // Character Access (by code point index)
    // ==========================================
    
    /// Get Unicode code point at specified position (with bounds checking)
    [[nodiscard]] char32_t at(size_t index) const;
    
    /// Get Unicode code point at specified position (no bounds checking)
    [[nodiscard]] char32_t operator[](size_t index) const;
    
    /// Get first code point
    [[nodiscard]] char32_t front() const;
    
    /// Get last code point
    [[nodiscard]] char32_t back() const;

    // ==========================================
    // Iterator (iterates by code point)
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
    // Operators
    // ==========================================
    
    /// Concatenation
    [[nodiscard]] UnicodeString operator+(const UnicodeString& other) const;
    [[nodiscard]] UnicodeString operator+(const std::string& utf8) const;
    [[nodiscard]] UnicodeString operator+(const char* utf8) const;
    [[nodiscard]] UnicodeString operator+(char32_t codepoint) const;
    
    UnicodeString& operator+=(const UnicodeString& other);
    UnicodeString& operator+=(const std::string& utf8);
    UnicodeString& operator+=(const char* utf8);
    UnicodeString& operator+=(char32_t codepoint);
    
    /// Comparison
    [[nodiscard]] bool operator==(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator!=(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator<(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator<=(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator>(const UnicodeString& other) const noexcept;
    [[nodiscard]] bool operator>=(const UnicodeString& other) const noexcept;
    
    /// Compare with std::string
    [[nodiscard]] bool operator==(const std::string& other) const noexcept;
    [[nodiscard]] bool operator==(const char* other) const noexcept;

    // ==========================================
    // Validation
    // ==========================================
    
    /// Check if internal data is valid UTF-8
    [[nodiscard]] bool isValid() const;
    
    /// Check if contains only ASCII characters
    [[nodiscard]] bool isAscii() const;
    
    /// Static validation methods
    [[nodiscard]] static bool isValidUtf8(const std::string& str);
    [[nodiscard]] static bool isValidUtf8(const char* str, size_t length);

    // ==========================================
    // Substring (by code point index)
    // ==========================================
    
    /// Get substring
    [[nodiscard]] UnicodeString substr(size_t pos, size_t count = npos) const;

    // ==========================================
    // Constants
    // ==========================================
    
    static constexpr size_t npos = static_cast<size_t>(-1);

private:
    std::string mData;  // Internal UTF-8 storage
    
    // Helper method
    [[nodiscard]] const char* pointerAt(size_t charIndex) const;
};

// ==========================================
// Non-member Functions
// ==========================================

/// Stream output
Utilities_EXPORT std::ostream& operator<<(std::ostream& os, const UnicodeString& str);

/// Concatenation operators
[[nodiscard]] Utilities_EXPORT UnicodeString operator+(const char* lhs, const UnicodeString& rhs);
[[nodiscard]] Utilities_EXPORT UnicodeString operator+(const std::string& lhs, const UnicodeString& rhs);

/// Comparison operators
[[nodiscard]] Utilities_EXPORT bool operator==(const std::string& lhs, const UnicodeString& rhs) noexcept;
[[nodiscard]] Utilities_EXPORT bool operator==(const char* lhs, const UnicodeString& rhs) noexcept;

// ==========================================
// Literal Support
// ==========================================

inline namespace literals {
    /// Literal suffix: "hello"_us
    [[nodiscard]] inline UnicodeString operator""_us(const char* str, size_t len)
    {
        return UnicodeString(std::string_view(str, len));
    }
}

} // namespace ucf::utilities
