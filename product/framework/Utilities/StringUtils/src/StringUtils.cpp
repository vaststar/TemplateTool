
#include <ucf/Utilities/StringUtils/StringUtils.h>
#include <ucf/Utilities/StringUtils/UnicodeString.h>

#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <charconv>
#include <sstream>

namespace ucf::utilities {

// ==========================================
// 空白处理 - std::string
// ==========================================

std::string StringUtils::trim(const std::string& str)
{
    return ltrim(rtrim(str));
}

std::string StringUtils::ltrim(const std::string& str)
{
    auto res = str;
    res.erase(res.begin(), std::find_if(res.begin(), res.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return res;
}

std::string StringUtils::rtrim(const std::string& str)
{
    auto res = str;
    res.erase(std::find_if(res.rbegin(), res.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), res.end());
    return res;
}

// ==========================================
// 空白处理 - UnicodeString
// ==========================================

UnicodeString StringUtils::trim(const UnicodeString& str)
{
    return UnicodeString(trim(str.toStdString()));
}

UnicodeString StringUtils::ltrim(const UnicodeString& str)
{
    return UnicodeString(ltrim(str.toStdString()));
}

UnicodeString StringUtils::rtrim(const UnicodeString& str)
{
    return UnicodeString(rtrim(str.toStdString()));
}

// ==========================================
// 大小写转换
// ==========================================

std::string StringUtils::toUpper(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return result;
}

std::string StringUtils::toLower(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

// ==========================================
// 分割和连接 - std::string
// ==========================================

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    std::istringstream stream(str);
    std::string token;
    
    while (std::getline(stream, token, delimiter)) {
        result.push_back(token);
    }
    
    return result;
}

std::vector<std::string> StringUtils::split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> result;
    
    if (delimiter.empty()) {
        result.push_back(str);
        return result;
    }
    
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    
    result.push_back(str.substr(start));
    return result;
}

std::string StringUtils::join(const std::vector<std::string>& parts, const std::string& delimiter)
{
    if (parts.empty()) return {};
    
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += delimiter + parts[i];
    }
    return result;
}

// ==========================================
// 分割和连接 - UnicodeString
// ==========================================

std::vector<UnicodeString> StringUtils::split(const UnicodeString& str, char delimiter)
{
    auto stdParts = split(str.toStdString(), delimiter);
    std::vector<UnicodeString> result;
    result.reserve(stdParts.size());
    
    for (const auto& part : stdParts) {
        result.emplace_back(part);
    }
    
    return result;
}

std::vector<UnicodeString> StringUtils::split(const UnicodeString& str, const UnicodeString& delimiter)
{
    auto stdParts = split(str.toStdString(), delimiter.toStdString());
    std::vector<UnicodeString> result;
    result.reserve(stdParts.size());
    
    for (const auto& part : stdParts) {
        result.emplace_back(part);
    }
    
    return result;
}

UnicodeString StringUtils::join(const std::vector<UnicodeString>& parts, const UnicodeString& delimiter)
{
    if (parts.empty()) return UnicodeString();
    
    UnicodeString result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += delimiter;
        result += parts[i];
    }
    return result;
}

// ==========================================
// 检查 - std::string
// ==========================================

bool StringUtils::startsWith(const std::string& str, const std::string& prefix)
{
    if (prefix.size() > str.size()) return false;
    return str.compare(0, prefix.size(), prefix) == 0;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix)
{
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool StringUtils::contains(const std::string& str, const std::string& substr)
{
    return str.find(substr) != std::string::npos;
}

// ==========================================
// 检查 - UnicodeString
// ==========================================

bool StringUtils::startsWith(const UnicodeString& str, const UnicodeString& prefix)
{
    return startsWith(str.toStdString(), prefix.toStdString());
}

bool StringUtils::endsWith(const UnicodeString& str, const UnicodeString& suffix)
{
    return endsWith(str.toStdString(), suffix.toStdString());
}

bool StringUtils::contains(const UnicodeString& str, const UnicodeString& substr)
{
    return contains(str.toStdString(), substr.toStdString());
}

// ==========================================
// 替换 - std::string
// ==========================================

std::string StringUtils::replace(const std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty()) return str;
    
    std::string result = str;
    size_t pos = result.find(from);
    
    if (pos != std::string::npos) {
        result.replace(pos, from.length(), to);
    }
    
    return result;
}

std::string StringUtils::replaceAll(const std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty()) return str;
    
    std::string result = str;
    size_t pos = 0;
    
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    
    return result;
}

// ==========================================
// 替换 - UnicodeString
// ==========================================

UnicodeString StringUtils::replace(const UnicodeString& str, const UnicodeString& from, const UnicodeString& to)
{
    return UnicodeString(replace(str.toStdString(), from.toStdString(), to.toStdString()));
}

UnicodeString StringUtils::replaceAll(const UnicodeString& str, const UnicodeString& from, const UnicodeString& to)
{
    return UnicodeString(replaceAll(str.toStdString(), from.toStdString(), to.toStdString()));
}

// ==========================================
// 数值转换
// ==========================================

std::optional<int> StringUtils::toInt(const std::string& str)
{
    if (str.empty()) return std::nullopt;
    
    int value = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    
    if (ec == std::errc() && ptr == str.data() + str.size()) {
        return value;
    }
    return std::nullopt;
}

std::optional<int64_t> StringUtils::toInt64(const std::string& str)
{
    if (str.empty()) return std::nullopt;
    
    int64_t value = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    
    if (ec == std::errc() && ptr == str.data() + str.size()) {
        return value;
    }
    return std::nullopt;
}

std::optional<double> StringUtils::toDouble(const std::string& str)
{
    if (str.empty()) return std::nullopt;
    
    try {
        size_t pos = 0;
        double value = std::stod(str, &pos);
        if (pos == str.size()) {
            return value;
        }
    } catch (...) {
        // 转换失败
    }
    return std::nullopt;
}

// ==========================================
// 其他工具
// ==========================================

std::string StringUtils::repeat(const std::string& str, size_t times)
{
    if (times == 0 || str.empty()) return {};
    
    std::string result;
    result.reserve(str.size() * times);
    
    for (size_t i = 0; i < times; ++i) {
        result += str;
    }
    
    return result;
}

std::string StringUtils::padLeft(const std::string& str, size_t width, char padChar)
{
    if (str.size() >= width) return str;
    return std::string(width - str.size(), padChar) + str;
}

std::string StringUtils::padRight(const std::string& str, size_t width, char padChar)
{
    if (str.size() >= width) return str;
    return str + std::string(width - str.size(), padChar);
}

} // namespace ucf::utilities