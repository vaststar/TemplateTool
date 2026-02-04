#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

// 前向声明
class UnicodeString;

/**
 * @brief 字符串工具类
 * 
 * 提供各种字符串操作的静态方法，支持 std::string 和 UnicodeString。
 */
class Utilities_EXPORT StringUtils final
{
public:
    // ==========================================
    // 空白处理 - std::string
    // ==========================================
    
    /// 去除两端空白
    static std::string trim(const std::string& str);
    
    /// 去除左侧空白
    static std::string ltrim(const std::string& str);
    
    /// 去除右侧空白
    static std::string rtrim(const std::string& str);
    
    // ==========================================
    // 空白处理 - UnicodeString
    // ==========================================
    
    static UnicodeString trim(const UnicodeString& str);
    static UnicodeString ltrim(const UnicodeString& str);
    static UnicodeString rtrim(const UnicodeString& str);

    // ==========================================
    // 大小写转换 - std::string (ASCII only)
    // ==========================================
    
    /// 转换为大写 (仅处理 ASCII 字符)
    static std::string toUpper(const std::string& str);
    
    /// 转换为小写 (仅处理 ASCII 字符)
    static std::string toLower(const std::string& str);

    // ==========================================
    // 分割和连接 - std::string
    // ==========================================
    
    /// 按单个字符分割
    static std::vector<std::string> split(const std::string& str, char delimiter);
    
    /// 按字符串分割
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    
    /// 连接字符串数组
    static std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

    // ==========================================
    // 分割和连接 - UnicodeString
    // ==========================================
    
    static std::vector<UnicodeString> split(const UnicodeString& str, char delimiter);
    static std::vector<UnicodeString> split(const UnicodeString& str, const UnicodeString& delimiter);
    static UnicodeString join(const std::vector<UnicodeString>& parts, const UnicodeString& delimiter);

    // ==========================================
    // 检查 - std::string
    // ==========================================
    
    /// 检查是否以指定前缀开头
    static bool startsWith(const std::string& str, const std::string& prefix);
    
    /// 检查是否以指定后缀结尾
    static bool endsWith(const std::string& str, const std::string& suffix);
    
    /// 检查是否包含子串
    static bool contains(const std::string& str, const std::string& substr);
    
    // ==========================================
    // 检查 - UnicodeString
    // ==========================================
    
    static bool startsWith(const UnicodeString& str, const UnicodeString& prefix);
    static bool endsWith(const UnicodeString& str, const UnicodeString& suffix);
    static bool contains(const UnicodeString& str, const UnicodeString& substr);

    // ==========================================
    // 替换 - std::string
    // ==========================================
    
    /// 替换第一个匹配
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
    
    /// 替换所有匹配
    static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);
    
    // ==========================================
    // 替换 - UnicodeString
    // ==========================================
    
    static UnicodeString replace(const UnicodeString& str, const UnicodeString& from, const UnicodeString& to);
    static UnicodeString replaceAll(const UnicodeString& str, const UnicodeString& from, const UnicodeString& to);

    // ==========================================
    // 数值转换
    // ==========================================
    
    /// 字符串转整数
    static std::optional<int> toInt(const std::string& str);
    
    /// 字符串转 64 位整数
    static std::optional<int64_t> toInt64(const std::string& str);
    
    /// 字符串转浮点数
    static std::optional<double> toDouble(const std::string& str);

    // ==========================================
    // 其他工具
    // ==========================================
    
    /// 重复字符串
    static std::string repeat(const std::string& str, size_t times);
    
    /// 左侧填充
    static std::string padLeft(const std::string& str, size_t width, char padChar = ' ');
    
    /// 右侧填充
    static std::string padRight(const std::string& str, size_t width, char padChar = ' ');

private:
    StringUtils() = delete;  // 禁止实例化
};

} // namespace ucf::utilities