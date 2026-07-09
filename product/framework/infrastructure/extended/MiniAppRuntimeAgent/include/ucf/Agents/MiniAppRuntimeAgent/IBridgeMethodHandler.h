#pragma once

#include <functional>
#include <string>
#include <vector>

#include <ucf/Agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentTypes.h>

namespace ucf::agents {

class IBridgeMethodHandler
{
public:
    using Reply = std::function<void(bool ok, const JsonValue& payload)>;

    virtual ~IBridgeMethodHandler() = default;

    [[nodiscard]] virtual std::string moduleName() const = 0;
    [[nodiscard]] virtual std::vector<std::string> requiredPermissions() const { return {}; }

    virtual void handle(const std::string& action, const JsonValue& params, Reply reply) = 0;
};

} // namespace ucf::agents
