#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>

#include <functional>

namespace MiniAppRuntime {

// A pluggable native capability, addressed as "<name>.<action>" from JS. Add
// capabilities by implementing this interface and registering it with the bridge.
class IJsApiModule
{
public:
    // Deliver the outcome of an invoke(). `ok == false` treats `payload` as an
    // error object (e.g. from makeError). May be called synchronously or later.
    using Reply = std::function<void(bool ok, const QJsonValue& payload)>;

    virtual ~IJsApiModule() = default;

    // Namespace prefix used in the JS method name, e.g. "system".
    [[nodiscard]] virtual QString name() const = 0;

    // Capability tokens the caller must have been granted. Empty => always allowed.
    [[nodiscard]] virtual QStringList requiredPermissions() const = 0;

    virtual void invoke(const QString& action, const QJsonObject& params, Reply reply) = 0;
};

} // namespace MiniAppRuntime
