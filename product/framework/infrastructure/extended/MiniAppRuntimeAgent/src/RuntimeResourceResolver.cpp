#include "RuntimeResourceResolver.h"

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

    const std::string ext = toLower(path.extension().string());
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
    const auto slashPos = rest.find('/');
    if (slashPos == std::string::npos)
    {
        hostOut = rest;
        pathOut.clear();
        return true;
    }

    hostOut = rest.substr(0, slashPos);
    pathOut = rest.substr(slashPos);
    return true;
}

} // namespace

RuntimeResourceResolver::RuntimeResourceResolver(std::string appId, std::string packageDir, std::string appScheme)
    : m_appId(std::move(appId))
    , m_schemePrefix((appScheme.empty() ? std::string("app") : std::move(appScheme)) + "://")
{
    std::error_code ec;
    m_packageRoot = std::filesystem::weakly_canonical(std::filesystem::path(packageDir), ec);
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

    const std::string relative = trimLeadingSlashes(rawPath);
    if (relative.empty())
    {
        result.statusCode = 404;
        return result;
    }

    std::error_code ec;
    const auto candidate = std::filesystem::weakly_canonical(m_packageRoot / relative, ec);
    if (ec)
    {
        result.statusCode = 404;
        return result;
    }

    const std::string rootText = m_packageRoot.generic_string();
    const std::string candidateText = candidate.generic_string();
    const std::string rootPrefix = rootText + "/";
    if (candidateText != rootText && candidateText.rfind(rootPrefix, 0) != 0)
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
