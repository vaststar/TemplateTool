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

    std::vector<std::string> grantedPermissions;
    std::vector<std::string> documentStartScripts;
    std::vector<std::string> additionalScriptChannels;

    bool allowNetworkByDefault = true;
    std::vector<std::string> allowedNetworkHosts;
};

} // namespace ucf::agents
