#pragma once

#include <string>

#include <ucf/Agents/MiniAppRuntimeAgent/IBridgeMethodHandler.h>

namespace commonHead::viewModels{

// Built-in "system" bridge module (no permissions). Reference handler owned by
// the runtime view model. All content is supplied by the view model layer at
// construction time, so this stays free of any Qt/platform dependency.
//   system.getInfo -> { platform, appId, appName, appVersion, sdkVersion }
class SystemBridgeHandler : public ucf::agents::IBridgeMethodHandler
{
public:
    struct Info
    {
        std::string appId;
        std::string appName;
        std::string appVersion;
        std::string platform;
    };

    explicit SystemBridgeHandler(Info info);

    [[nodiscard]] std::string moduleName() const override;
    void handle(const std::string& action,
                const ucf::agents::JsonValue& params,
                Reply reply) override;

private:
    Info mInfo;
};
}
