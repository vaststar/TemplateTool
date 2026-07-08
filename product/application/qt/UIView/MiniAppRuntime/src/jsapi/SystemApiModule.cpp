#include "jsapi/SystemApiModule.h"

#include <QJsonObject>
#include <QLocale>
#include <QSysInfo>

#include "MiniAppMessage.h"

namespace MiniAppRuntime {

namespace {
constexpr const char* kSdkVersion = "1.0.0";
}

SystemApiModule::SystemApiModule(QString appId, QString appName)
    : m_appId(std::move(appId))
    , m_appName(std::move(appName))
{
}

QString SystemApiModule::name() const
{
    return QStringLiteral("system");
}

QStringList SystemApiModule::requiredPermissions() const
{
    return {};
}

void SystemApiModule::invoke(const QString& action, const QJsonObject& params, Reply reply)
{
    Q_UNUSED(params);

    if (action == QStringLiteral("getInfo"))
    {
        QJsonObject info;
        info.insert(QStringLiteral("platform"), QSysInfo::productType());
        info.insert(QStringLiteral("arch"), QSysInfo::currentCpuArchitecture());
        info.insert(QStringLiteral("appId"), m_appId);
        info.insert(QStringLiteral("appName"), m_appName);
        info.insert(QStringLiteral("locale"), QLocale::system().name());
        info.insert(QStringLiteral("sdkVersion"), QString::fromLatin1(kSdkVersion));
        reply(true, info);
        return;
    }

    reply(false, makeError(QStringLiteral("unknown action: system.") + action));
}

} // namespace MiniAppRuntime
