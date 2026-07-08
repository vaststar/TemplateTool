#pragma once

#include "jsapi/IJsApiModule.h"

namespace MiniAppRuntime {

// Built-in "system" module (no permissions). Reference implementation.
//   system.getInfo -> { platform, arch, appId, appName, locale, sdkVersion }
class SystemApiModule : public IJsApiModule
{
public:
    SystemApiModule(QString appId, QString appName);

    [[nodiscard]] QString name() const override;
    [[nodiscard]] QStringList requiredPermissions() const override;
    void invoke(const QString& action, const QJsonObject& params, Reply reply) override;

private:
    QString m_appId;
    QString m_appName;
};

} // namespace MiniAppRuntime
