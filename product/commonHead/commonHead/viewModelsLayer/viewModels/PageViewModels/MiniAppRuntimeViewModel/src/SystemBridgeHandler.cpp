#include "SystemBridgeHandler.h"

namespace commonHead::viewModels{

namespace {
constexpr const char* kSdkVersion = "1.0.0";
}

SystemBridgeHandler::SystemBridgeHandler(Info info)
    : mInfo(std::move(info))
{
}

std::string SystemBridgeHandler::moduleName() const
{
    return "system";
}

void SystemBridgeHandler::handle(const std::string& action,
                                 const ucf::agents::JsonValue& params,
                                 Reply reply)
{
    (void)params;

    if (action == "getInfo")
    {
        ucf::agents::JsonValue info = ucf::agents::JsonValue::object();
        info.set("platform", mInfo.platform);
        info.set("appId", mInfo.appId);
        info.set("appName", mInfo.appName);
        info.set("appVersion", mInfo.appVersion);
        info.set("sdkVersion", std::string(kSdkVersion));
        reply(true, info);
        return;
    }

    ucf::agents::JsonValue err = ucf::agents::JsonValue::object();
    err.set("message", std::string("unknown action: system.") + action);
    reply(false, err);
}
}
