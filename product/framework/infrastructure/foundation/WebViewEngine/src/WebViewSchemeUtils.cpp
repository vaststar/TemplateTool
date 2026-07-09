#include "WebViewSchemeUtils.h"

#include <algorithm>

namespace ucf::infrastructure::webview::scheme_utils {

bool isValidCustomScheme(const std::string& scheme)
{
    if (scheme.empty())
    {
        return false;
    }
    auto isSchemeChar = [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
               c == '+' || c == '.' || c == '-';
    };
    if (!(scheme.front() >= 'a' && scheme.front() <= 'z'))
    {
        return false;
    }
    for (char c : scheme)
    {
        if (!isSchemeChar(c))
        {
            return false;
        }
    }
    static const char* const kReserved[] = {
        "http", "https", "file", "ftp", "ws", "wss",
        "about", "data", "blob", "javascript"
    };
    for (const char* reserved : kReserved)
    {
        if (scheme == reserved)
        {
            return false;
        }
    }
    return true;
}

std::optional<std::vector<std::string>> normalizeCustomSchemes(
    const std::vector<std::string>& requested)
{
    std::vector<std::string> schemes;
    for (const std::string& scheme : requested)
    {
        if (!isValidCustomScheme(scheme))
        {
            return std::nullopt;
        }
        if (std::find(schemes.cbegin(), schemes.cend(), scheme) == schemes.cend())
        {
            schemes.push_back(scheme);
        }
    }
    if (schemes.empty())
    {
        schemes.emplace_back("app");
    }
    return schemes;
}

} // namespace ucf::infrastructure::webview::scheme_utils
