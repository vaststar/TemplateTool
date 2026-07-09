#pragma once

#include <QJsonValue>
#include <QObject>
#include <QString>

#include <memory>
#include <vector>

#include <ucf/Agents/MiniAppRuntimeAgent/MiniAppBridgeCore.h>

namespace MiniAppRuntime {

class IMiniAppWebView;
class IJsApiModule;
class PermissionGate;
class QtBridgeTransportAdapter;

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
    IMiniAppWebView* m_webView = nullptr; // not owned
    PermissionGate* m_gate = nullptr;     // not owned
    std::unique_ptr<QtBridgeTransportAdapter> m_transport;
    std::vector<std::shared_ptr<ucf::agents::IBridgeMethodHandler>> m_handlerAdapters;
    ucf::agents::MiniAppBridgeCore m_core;
};

} // namespace MiniAppRuntime
