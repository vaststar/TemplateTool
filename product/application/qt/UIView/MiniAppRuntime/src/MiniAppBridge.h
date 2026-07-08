#pragma once

#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>

#include <memory>

namespace MiniAppRuntime {

class IMiniAppWebView;
class IJsApiModule;
class PermissionGate;

// JS <-> C++ dispatch hub for one mini-app: parses invoke messages, checks
// permissions, routes to the matching JSAPI module and returns callbacks. Owns modules.
class MiniAppBridge : public QObject
{
    Q_OBJECT
public:
    explicit MiniAppBridge(QObject* parent = nullptr);
    ~MiniAppBridge() override;

    // `gate` is not owned; it must outlive the bridge.
    void setPermissionGate(PermissionGate* gate);

    // Takes ownership of `module`.
    void registerModule(std::unique_ptr<IJsApiModule> module);

    // Subscribe to the view's messageReceived signal. Does not take ownership.
    void attach(IMiniAppWebView* webView);

    // Push an unsolicited event to the page.
    void emitEvent(const QString& event, const QJsonValue& data);

private slots:
    void onMessageReceived(const QString& json);

private:
    void dispatchInvoke(qint64 id, const QString& method, const QJsonObject& params);
    void sendCallback(qint64 id, bool ok, const QJsonValue& payload);
    void dispatchToJs(const QString& json);

    IMiniAppWebView* m_webView = nullptr; // not owned
    PermissionGate* m_gate = nullptr;     // not owned
    QHash<QString, std::shared_ptr<IJsApiModule>> m_modules;
};

} // namespace MiniAppRuntime
