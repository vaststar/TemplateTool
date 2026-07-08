#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace MiniAppRuntime {

// Wire format between the injected JS SDK and the native runtime.
//
// Web -> Native (invoke):
//   { "type":"invoke", "id":<int>, "method":"<module>.<action>", "params":{...} }
// Native -> Web (callback):
//   { "type":"callback", "id":<int>, "ok":<bool>, "result":<any>|"error":{...} }
// Native -> Web (event, unsolicited):
//   { "type":"event", "event":"<name>", "data":<any> }

// Parsed representation of a Web -> Native "invoke" message.
struct InboundMessage
{
    bool valid = false;
    qint64 id = -1;      // callback correlation id
    QString method;      // "<module>.<action>"
    QJsonObject params;  // may be empty
};

inline InboundMessage parseInbound(const QString& json)
{
    InboundMessage msg;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject())
    {
        return msg;
    }
    const QJsonObject obj = doc.object();
    if (obj.value(QStringLiteral("type")).toString() != QStringLiteral("invoke"))
    {
        return msg;
    }
    msg.id = static_cast<qint64>(obj.value(QStringLiteral("id")).toDouble(-1));
    msg.method = obj.value(QStringLiteral("method")).toString();
    msg.params = obj.value(QStringLiteral("params")).toObject();
    msg.valid = !msg.method.isEmpty();
    return msg;
}

inline QJsonObject makeError(const QString& message)
{
    QJsonObject err;
    err.insert(QStringLiteral("message"), message);
    return err;
}

inline QString buildCallback(qint64 id, bool ok, const QJsonValue& payload)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("type"), QStringLiteral("callback"));
    obj.insert(QStringLiteral("id"), static_cast<double>(id));
    obj.insert(QStringLiteral("ok"), ok);
    obj.insert(ok ? QStringLiteral("result") : QStringLiteral("error"), payload);
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

inline QString buildEvent(const QString& event, const QJsonValue& data)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("type"), QStringLiteral("event"));
    obj.insert(QStringLiteral("event"), event);
    obj.insert(QStringLiteral("data"), data);
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

} // namespace MiniAppRuntime
