#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

// Forward declaration
class UnicodeString;

/**
 * @brief String utility class
 * 
 * Provides static methods for various string operations, supporting both std::string and UnicodeString.
 */
class Utilities_EXPORT StringUtils final
{
public:
    // ==========================================
    // Whitespace Handling - std::string
    // ==========================================
    
    /// Trim leading and trailing whitespace
    static std::string trim(const std::string& str);
    
    /// Trim leading whitespace
    static std::string ltrim(const std::string& str);
    
    /// Trim trailing whitespace
    static std::string rtrim(const std::string& str);
    
    // ==========================================
    // Whitespace Handling - UnicodeString
    // ==========================================
    
    static UnicodeString trim(const UnicodeString& str);
    static UnicodeString ltrim(const UnicodeString& str);
    static UnicodeString rtrim(const UnicodeString& str);

    // ==========================================
    // Case Conversion - std::string (ASCII only)
    // ==========================================
    
    /// Convert to uppercase (ASCII characters only)
    static std::string toUpper(const std::string& str);
    
    /// Convert to lowercase (ASCII characters only)
    static std::string toLower(const std::string& str);

    // ==========================================
    // Split and Join - std::string
    // ==========================================
    
    /// Split by single character
    static std::vector<std::string> split(const std::string& str, char delimiter);
    
    /// Split by string
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    
    /// Join string array
    static std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

    // ==========================================
    // Split and Join - UnicodeString
    // ==========================================
    
    static std::vector<UnicodeString> split(const UnicodeString& str, char delimiter);
    static std::vector<UnicodeString> split(const UnicodeString& str, const UnicodeString& delimiter);
    static UnicodeString join(const std::vector<UnicodeString>& parts, const UnicodeString& delimiter);

    // ==========================================
    // Checks - std::string
    // ==========================================
    
    /// Check if starts with prefix
    static bool startsWith(const std::string& str, const std::string& prefix);
    
    /// Check if ends with suffix
    static bool endsWith(const std::string& str, const std::string& suffix);
    
    /// Check if contains substring
    static bool contains(const std::string& str, const std::string& substr);
    
    // ==========================================
    // Checks - UnicodeString
    // ==========================================
    
    static bool startsWith(const UnicodeString& str, const UnicodeString& prefix);
    static bool endsWith(const UnicodeString& str, const UnicodeString& suffix);
    static bool contains(const UnicodeString& str, const UnicodeString& substr);

    // ==========================================
    // Replace - std::string
    // ==========================================
    
    /// Replace first match
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
    
    /// Replace all matches
    static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);
    
    // ==========================================
    // Replace - UnicodeString
    // ==========================================
    
    static UnicodeString replace(const UnicodeString& str, const UnicodeString& from, const UnicodeString& to);
    static UnicodeString replaceAll(const UnicodeString& str, const UnicodeString& from, const UnicodeString& to);

    // ==========================================
    // Numeric Conversion
    // ==========================================
    
    /// Convert string to integer
    static std::optional<int> toInt(const std::string& str);
    
    /// Convert string to 64-bit integer
    static std::optional<int64_t> toInt64(const std::string& str);
    
    /// Convert string to double
    static std::optional<double> toDouble(const std::string& str);

    // ==========================================
    // Other Utilities
    // ==========================================
    
    /// Repeat string
    static std::string repeat(const std::string& str, size_t times);
    
    /// Pad left
    static std::string padLeft(const std::string& str, size_t width, char padChar = ' ');
    
    /// Pad right
    static std::string padRight(const std::string& str, size_t width, char padChar = ' ');

private:
    StringUtils() = delete;  // Prevent instantiation
};

} // namespace ucf::utilities