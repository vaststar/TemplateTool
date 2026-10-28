#include <ucf/infrastructure/NetworkClient/LibCurlClient/HttpRedirect.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpRequest.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include <curl/curl.h>

namespace ucf::infrastructure::network::libcurl {
namespace {

using CurlUrl = std::unique_ptr<CURLU, decltype(&curl_url_cleanup)>;
using CurlString = std::unique_ptr<char, decltype(&curl_free)>;

bool equalsAsciiIgnoringCase(std::string_view first, std::string_view second) noexcept
{
    const auto lower = [](unsigned char value) {
        return value >= 'A' && value <= 'Z' ? value + ('a' - 'A') : value;
    };
    return first.size() == second.size()
        && std::equal(first.begin(), first.end(), second.begin(),
            [&](unsigned char left, unsigned char right) { return lower(left) == lower(right); });
}

std::optional<std::string> getUrlPart(CURLU* url, CURLUPart part, unsigned int flags = 0)
{
    char* value = nullptr;
    const auto code = curl_url_get(url, part, &value, flags);
    CurlString owned(value, &curl_free);
    if (code != CURLUE_OK || !owned)
    {
        return std::nullopt;
    }
    return std::string(owned.get());
}

struct Origin {
    std::string scheme;
    std::string host;
    unsigned int port;
    std::string zoneId;
};

std::optional<Origin> getOrigin(CURLU* url)
{
    auto scheme = getUrlPart(url, CURLUPART_SCHEME);
    auto host = getUrlPart(url, CURLUPART_HOST, CURLU_PUNYCODE);
    auto port = getUrlPart(url, CURLUPART_PORT, CURLU_DEFAULT_PORT);
    if (!scheme || !host || !port
        || (!equalsAsciiIgnoringCase(*scheme, "http") && !equalsAsciiIgnoringCase(*scheme, "https")))
    {
        return std::nullopt;
    }
    unsigned int portNumber = 0;
    const auto parsed = std::from_chars(port->data(), port->data() + port->size(), portNumber);
    if (parsed.ec != std::errc{} || parsed.ptr != port->data() + port->size())
    {
        return std::nullopt;
    }

    // CURLUPART_HOST excludes the IPv6 interface scope identifier.
    char* zone = nullptr;
    const auto zoneCode = curl_url_get(url, CURLUPART_ZONEID, &zone, 0);
    CurlString ownedZone(zone, &curl_free);
    if (zoneCode != CURLUE_OK && zoneCode != CURLUE_NO_ZONEID)
    {
        return std::nullopt;
    }
    return Origin{*scheme, *host, portNumber, ownedZone ? ownedZone.get() : ""};
}

bool sameOrigin(const Origin& first, const Origin& second) noexcept
{
    return equalsAsciiIgnoringCase(first.scheme, second.scheme)
        && equalsAsciiIgnoringCase(first.host, second.host)
        && first.port == second.port
        && first.zoneId == second.zoneId;
}

bool isCrossOriginHeader(std::string_view name) noexcept
{
    constexpr std::array<std::string_view, 4> headers{
        "Authorization", "Cookie", "Proxy-Authorization", "Host"
    };
    return std::any_of(headers.begin(), headers.end(),
        [&](std::string_view header) { return equalsAsciiIgnoringCase(name, header); });
}

bool isPayloadHeader(std::string_view name) noexcept
{
    constexpr std::array<std::string_view, 10> headers{
        "Content-Encoding", "Content-Language", "Content-Location", "Content-Type",
        "Content-Length", "Digest", "Last-Modified", "Transfer-Encoding", "Expect", "Trailer"
    };
    return std::any_of(headers.begin(), headers.end(),
        [&](std::string_view header) { return equalsAsciiIgnoringCase(name, header); });
}

} // namespace

bool tryApplyHttpRedirect(http::NetworkHttpRequest& request, int statusCode,
                          const http::NetworkHttpHeaders& responseHeaders)
{
    if (statusCode != 301 && statusCode != 302 && statusCode != 303
        && statusCode != 307 && statusCode != 308)
    {
        return false;
    }

    const auto locationHeader = std::find_if(responseHeaders.begin(), responseHeaders.end(),
        [](const auto& header) { return equalsAsciiIgnoringCase(header.first, "Location"); });
    if (locationHeader == responseHeaders.end()
        || locationHeader->second.find('\0') != std::string::npos
        || request.getRequestUri().find('\0') != std::string::npos)
    {
        return false;
    }
    const std::string_view locationValue(locationHeader->second);
    const auto first = locationValue.find_first_not_of(" \t");
    if (first == std::string_view::npos)
    {
        return false;
    }
    const auto last = locationValue.find_last_not_of(" \t");
    const std::string location(locationValue.substr(first, last - first + 1));

    CurlUrl url(curl_url(), &curl_url_cleanup);
    if (!url || curl_url_set(url.get(), CURLUPART_URL, request.getRequestUri().c_str(), 0) != CURLUE_OK)
    {
        return false;
    }
    const auto sourceOrigin = getOrigin(url.get());
    if (!sourceOrigin || curl_url_set(url.get(), CURLUPART_URL, location.c_str(), 0) != CURLUE_OK)
    {
        return false;
    }
    const auto targetOrigin = getOrigin(url.get());
    if (!targetOrigin)
    {
        return false;
    }
    const bool crossOrigin = !sameOrigin(*sourceOrigin, *targetOrigin);
    if (crossOrigin
        && (curl_url_set(url.get(), CURLUPART_USER, nullptr, 0) != CURLUE_OK
            || curl_url_set(url.get(), CURLUPART_PASSWORD, nullptr, 0) != CURLUE_OK
            || curl_url_set(url.get(), CURLUPART_OPTIONS, nullptr, 0) != CURLUE_OK))
    {
        return false;
    }
    const auto targetUrl = getUrlPart(url.get(), CURLUPART_URL, CURLU_GET_EMPTY);
    if (!targetUrl)
    {
        return false;
    }

    const auto oldMethod = request.getRequestMethod();
    const bool postToGet = (statusCode == 301 || statusCode == 302) && oldMethod == http::HTTPMethod::POST;
    const bool dropPayload = statusCode == 303 || postToGet;
    auto method = oldMethod;
    if (dropPayload && oldMethod != http::HTTPMethod::HEAD)
    {
        method = http::HTTPMethod::GET;
    }
    auto headers = request.getRequestHeaders();
    std::erase_if(headers, [&](const auto& header) {
        return (crossOrigin && isCrossOriginHeader(header.first))
            || (dropPayload && isPayloadHeader(header.first));
    });

    // All ordinary validation failures return above. Preserve identity and timeout.
    request.setRequestUri(*targetUrl);
    request.setRequestHeaders(headers);
    request.setRequestMethod(method);
    if (dropPayload)
    {
        request.clearPayload();
    }
    return true;
}

} // namespace ucf::infrastructure::network::libcurl
