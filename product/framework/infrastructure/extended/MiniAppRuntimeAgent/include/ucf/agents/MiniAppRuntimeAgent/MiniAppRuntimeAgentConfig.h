#pragma once

#include <string>
#include <vector>

namespace ucf::agents {

struct MiniAppRuntimeAgentConfig
{
    std::string appId;
    std::string packageDir;
    std::string entry = "index.html";
    std::string appScheme = "app";
    std::string bridgeChannel = "miniapp";
    // Writable directory for the web backend's per-app browsing data (cache,
    // cookies, local storage). Required by the Windows WebView2 backend; ignored
    // by backends that manage their own store (macOS WKWebView). Empty means the
    // backend default. Typically the mini-app's per-app cache directory.
    std::string userDataFolder;

    std::vector<std::string> grantedPermissions;
    std::vector<std::string> documentStartScripts;

    bool allowNetworkByDefault = true;
    std::vector<std::string> allowedNetworkHosts;
};

} // namespace ucf::agents
