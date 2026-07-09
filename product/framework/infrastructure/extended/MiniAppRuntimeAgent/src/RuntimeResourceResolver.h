#pragma once

#include <filesystem>
#include <string>

#include <ucf/Agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentTypes.h>

namespace ucf::agents {

// Resolves app://<appId>/<path> requests to files inside the mini-app package,
// with path-traversal protection. Internal assembly part of the runtime agent.
class RuntimeResourceResolver
{
public:
    RuntimeResourceResolver(std::string appId, std::string packageDir, std::string appScheme = "app");

    [[nodiscard]] bool canHandle(const std::string& url) const;
    [[nodiscard]] ResolvedResource resolve(const std::string& url) const;

private:
    std::string m_appId;
    std::string m_schemePrefix; // e.g. "app://"
    std::filesystem::path m_packageRoot;
};

} // namespace ucf::agents
