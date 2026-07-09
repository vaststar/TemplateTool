#pragma once

#include <string>

#include <ucf/Agents/MiniAppRuntimeAgent/IBridgeMethodHandler.h>

namespace MiniAppRuntime {

// Built-in "system" bridge module (no permissions). Reference handler wired into
// the runtime agent by MiniAppSession.
//   system.getInfo -> { platform, arch, appId, appName, locale, sdkVersion }
class SystemApiModule : public ucf::agents::IBridgeMethodHandler
{
public:
    SystemApiModule(std::string appId, std::string appName);

    [[nodiscard]] std::string moduleName() const override;
    void handle(const std::string& action,
                const ucf::agents::JsonValue& params,
                Reply reply) override;

private:
    std::string m_appId;
    std::string m_appName;
};

} // namespace MiniAppRuntime
