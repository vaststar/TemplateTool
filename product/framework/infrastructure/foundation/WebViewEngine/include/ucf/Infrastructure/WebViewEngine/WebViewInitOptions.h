#pragma once

#include <optional>
#include <string>
#include <vector>

#include <ucf/Infrastructure/WebViewEngine/WebViewTypes.h>

namespace ucf::infrastructure::webview {

struct WebViewInitOptions
{
    // Custom schemes routed to the request-interceptor chain (e.g. "app",
    // "myapp"). Used both for local resource loading and for catching
    // custom-scheme redirects (SSO callbacks like myapp://callback?code=...).
    // Reserved schemes (http/https/file/ftp/ws/wss/about/data/blob/javascript)
    // are rejected. Must be provided at init; schemes cannot be added after the
    // web view is created. Empty means the backend default ("app").
    std::vector<std::string> customSchemes;
    std::vector<std::string> documentStartScripts;
    std::vector<std::string> scriptChannels;
    // Full User-Agent override. std::nullopt keeps the backend default UA.
    std::optional<std::string> userAgentOverride;
    bool allowPopups = false;
    // Declarative network access policy enforced at the network layer. When set,
    // the engine compiles the rules during initialize() and only reports
    // onWebViewReady after they are installed (fail-closed on error).
    // std::nullopt means no restriction. Applies only to remote protocols.
    std::optional<NetworkAccessPolicy> networkPolicy;
};

} // namespace ucf::infrastructure::webview
