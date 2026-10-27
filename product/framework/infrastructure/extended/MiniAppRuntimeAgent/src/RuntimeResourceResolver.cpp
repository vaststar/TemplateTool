#include "RuntimeResourceResolver.h"

#include <ucf/utilities/FilePathUtils/FilePathUtils.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <map>
#include <system_error>
#include <utility>
#include <vector>

namespace ucf::agents {
namespace {

bool startsWith(const std::string& text, const std::string& prefix)
{
    return text.rfind(prefix, 0) == 0;
}

std::string trimLeadingSlashes(const std::string& input)
{
    size_t pos = 0;
    while (pos < input.size() && input[pos] == '/')
    {
        ++pos;
    }
    return input.substr(pos);
}

int hexDigitValue(char ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

bool decodeUrlPath(const std::string& rawPath, std::string& decodedPath)
{
    decodedPath.clear();
    decodedPath.reserve(rawPath.size());
    for (size_t i = 0; i < rawPath.size(); ++i)
    {
        unsigned char ch = static_cast<unsigned char>(rawPath[i]);
        if (ch == '%')
        {
            if (rawPath.size() - i < 3) return false;
            const int high = hexDigitValue(rawPath[i + 1]);
            const int low = hexDigitValue(rawPath[i + 2]);
            if (high < 0 || low < 0) return false;
            ch = static_cast<unsigned char>((high << 4) | low);
            i += 2;
        }
        if (ch == 0) return false;
        decodedPath.push_back(static_cast<char>(ch));
    }
    return true;
}

std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string mimeTypeForPath(const std::filesystem::path& path)
{
    static const std::map<std::string, std::string> kMimeMap = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".js", "text/javascript"},
        {".mjs", "text/javascript"},
        {".css", "text/css"},
        {".json", "application/json"},
        {".svg", "image/svg+xml"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".txt", "text/plain"},
    };

    const std::string ext = toLower(
        ucf::utilities::FilePathUtils::utf8FromPath(path.extension()));
    const auto it = kMimeMap.find(ext);
    if (it != kMimeMap.end())
    {
        return it->second;
    }
    return "application/octet-stream";
}

bool parseAppUrl(const std::string& url, const std::string& schemePrefix, std::string& hostOut, std::string& pathOut)
{
    if (!startsWith(url, schemePrefix))
    {
        return false;
    }

    const std::string rest = url.substr(schemePrefix.size());
    const auto authorityEnd = rest.find_first_of("/?#");
    hostOut = rest.substr(0, authorityEnd);
    pathOut.clear();
    if (hostOut.empty())
    {
        return false;
    }
    if (authorityEnd == std::string::npos || rest[authorityEnd] != '/')
    {
        return true;
    }

    // Split URL components before decoding, so %3F and %23 stay in the path.
    const auto pathEnd = rest.find_first_of("?#", authorityEnd);
    pathOut = rest.substr(authorityEnd, pathEnd == std::string::npos
        ? std::string::npos : pathEnd - authorityEnd);
    return true;
}

} // namespace

RuntimeResourceResolver::RuntimeResourceResolver(std::string appId, std::string packageDir, std::string appScheme)
    : m_appId(std::move(appId))
    , m_schemePrefix((appScheme.empty() ? std::string("app") : std::move(appScheme)) + "://")
{
    std::error_code ec;
    m_packageRoot = std::filesystem::weakly_canonical(
        ucf::utilities::FilePathUtils::pathFromUtf8(packageDir), ec);
    if (ec)
    {
        m_packageRoot.clear();
    }
}

bool RuntimeResourceResolver::canHandle(const std::string& url) const
{
    return startsWith(url, m_schemePrefix);
}

ResolvedResource RuntimeResourceResolver::resolve(const std::string& url) const
{
    ResolvedResource result;
    result.handled = true;

    if (m_packageRoot.empty())
    {
        result.statusCode = 500;
        result.errorMessage = "invalid package root";
        return result;
    }

    std::string host;
    std::string rawPath;
    if (!parseAppUrl(url, m_schemePrefix, host, rawPath))
    {
        result.statusCode = 400;
        result.errorMessage = "invalid app url";
        return result;
    }

    if (host != m_appId)
    {
        result.statusCode = 403;
        result.errorMessage = "cross-app access denied";
        return result;
    }

    std::string relative;
    // Strip URL separators before decoding; never decode the resulting path twice.
    if (!decodeUrlPath(trimLeadingSlashes(rawPath), relative))
    {
        result.statusCode = 400;
        result.errorMessage = "invalid resource path";
        return result;
    }
    if (relative.empty())
    {
        result.statusCode = 404;
        return result;
    }

    const auto relativePath = ucf::utilities::FilePathUtils::pathFromUtf8(relative);
    if (relativePath.has_root_path())
    {
        result.statusCode = 403;
        result.errorMessage = "path traversal denied";
        return result;
    }

    std::error_code ec;
    const auto candidate = std::filesystem::weakly_canonical(m_packageRoot / relativePath, ec);
    if (ec)
    {
        result.statusCode = 404;
        return result;
    }

    const auto mismatch = std::mismatch(
        m_packageRoot.begin(), m_packageRoot.end(), candidate.begin(), candidate.end());
    if (mismatch.first != m_packageRoot.end())
    {
        result.statusCode = 403;
        result.errorMessage = "path traversal denied";
        return result;
    }

    if (!std::filesystem::exists(candidate) || !std::filesystem::is_regular_file(candidate))
    {
        result.statusCode = 404;
        return result;
    }

    std::ifstream input(candidate, std::ios::binary);
    if (!input)
    {
        result.statusCode = 500;
        result.errorMessage = "failed to open resource";
        return result;
    }

    result.body = std::vector<std::uint8_t>(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    result.ok = true;
    result.statusCode = 200;
    result.mimeType = mimeTypeForPath(candidate);
    result.headers["Cache-Control"] = "no-cache";
    return result;
}

} // namespace ucf::agents
