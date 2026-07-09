#include "jsapi/SystemApiModule.h"

#include <QLocale>
#include <QSysInfo>

namespace MiniAppRuntime {

namespace {
constexpr const char* kSdkVersion = "1.0.0";
}

SystemApiModule::SystemApiModule(std::string appId, std::string appName)
    : m_appId(std::move(appId))
    , m_appName(std::move(appName))
{
}

std::string SystemApiModule::moduleName() const
{
    return "system";
}

void SystemApiModule::handle(const std::string& action,
                             const ucf::agents::JsonValue& params,
                             Reply reply)
{
    (void)params;

    if (action == "getInfo")
    {
        ucf::agents::JsonValue info = ucf::agents::JsonValue::object();
        info.set("platform", QSysInfo::productType().toStdString());
        info.set("arch", QSysInfo::currentCpuArchitecture().toStdString());
        info.set("appId", m_appId);
        info.set("appName", m_appName);
        info.set("locale", QLocale::system().name().toStdString());
        info.set("sdkVersion", std::string(kSdkVersion));
        reply(true, info);
        return;
    }

    ucf::agents::JsonValue err = ucf::agents::JsonValue::object();
    err.set("message", std::string("unknown action: system.") + action);
    reply(false, err);
}

} // namespace MiniAppRuntime
