#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace ucf::infrastructure::webview {

using NativeHostHandle = std::uintptr_t;
using InterceptorId = std::uint64_t;

struct WebError
{
    int code = 0;
    std::string message;
};

// onLoadFailed 'code' used when the network access-control rules fail to
// compile/install. The web view stays fail-closed (never becomes ready) so no
// content is loaded without the policy in force.
inline constexpr int kWebErrorContentPolicyCompileFailed = -2001;

// Declarative network access policy enforced by the engine at the network layer
// (WKContentRuleList on macOS). Only remote protocols (http/https/ws/wss) are
// affected; local schemes (custom app scheme, about:, data:, blob:) are never
// blocked. std::nullopt on WebViewInitOptions means "no restriction".
struct NetworkAccessPolicy
{
    enum class DefaultAction
    {
        Allow, // remote requests not matching allowedHosts are permitted
        Block  // remote requests not matching allowedHosts are blocked
    };

    // Action for remote requests that do not match allowedHosts/blockedHosts.
    DefaultAction defaultAction = DefaultAction::Allow;

    // Hosts always allowed (lower-case, e.g. "api.example.com").
    std::vector<std::string> allowedHosts;

    // Hosts always blocked; takes precedence even when defaultAction is Allow.
    std::vector<std::string> blockedHosts;

    // Whether host entries also match their subdomains
    // (api.example.com also covers x.api.example.com).
    bool includeSubdomains = true;
};

struct WebRequest
{
    std::string url;
    std::string method;
    std::map<std::string, std::string> headers;
    std::vector<std::uint8_t> body;
};

struct WebResponse
{
    int statusCode = 200;
    std::string mimeType;
    std::map<std::string, std::string> headers;
    std::vector<std::uint8_t> body;
};

enum class InterceptAction
{
    Continue,
    Respond,
    Fail
};

struct InterceptResult
{
    InterceptAction action = InterceptAction::Continue;
    std::optional<WebResponse> response;
    std::optional<WebError> error;
};

// Result callback for asynchronous JavaScript evaluation.
// success=true: jsonResult holds the JSON-encoded return value; error unset.
// success=false: error holds the failure and jsonResult is empty.
// Invoked on the main/UI thread.
using JavaScriptResultCallback =
    std::function<void(bool success, const std::string& jsonResult, const WebError& error)>;

} // namespace ucf::infrastructure::webview
