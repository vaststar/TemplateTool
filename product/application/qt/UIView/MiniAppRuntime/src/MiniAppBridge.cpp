#include "MiniAppBridge.h"

#include <QJsonDocument>

#include "MiniAppMessage.h"
#include "PermissionGate.h"
#include "jsapi/IJsApiModule.h"
#include "webview/IMiniAppWebView.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace MiniAppRuntime {

MiniAppBridge::MiniAppBridge(QObject* parent)
    : QObject(parent)
{
}

MiniAppBridge::~MiniAppBridge() = default;

void MiniAppBridge::setPermissionGate(PermissionGate* gate)
{
    m_gate = gate;
}

void MiniAppBridge::registerModule(std::unique_ptr<IJsApiModule> module)
{
    if (!module)
    {
        return;
    }
    const QString name = module->name();
    m_modules.insert(name, std::shared_ptr<IJsApiModule>(std::move(module)));
}

void MiniAppBridge::attach(IMiniAppWebView* webView)
{
    if (m_webView == webView)
    {
        return;
    }
    if (m_webView)
    {
        disconnect(m_webView, &IMiniAppWebView::messageReceived, this, &MiniAppBridge::onMessageReceived);
    }
    m_webView = webView;
    if (m_webView)
    {
        connect(m_webView, &IMiniAppWebView::messageReceived, this, &MiniAppBridge::onMessageReceived);
    }
}

void MiniAppBridge::emitEvent(const QString& event, const QJsonValue& data)
{
    dispatchToJs(buildEvent(event, data));
}

void MiniAppBridge::onMessageReceived(const QString& json)
{
    const InboundMessage msg = parseInbound(json);
    if (!msg.valid)
    {
        UIVIEW_LOG_WARN("MiniAppBridge: ignoring malformed message");
        return;
    }
    dispatchInvoke(msg.id, msg.method, msg.params);
}

void MiniAppBridge::dispatchInvoke(qint64 id, const QString& method, const QJsonObject& params)
{
    const int dot = method.indexOf(QLatin1Char('.'));
    if (dot <= 0)
    {
        sendCallback(id, false, makeError(QStringLiteral("invalid method: ") + method));
        return;
    }
    const QString moduleName = method.left(dot);
    const QString action = method.mid(dot + 1);

    const auto it = m_modules.constFind(moduleName);
    if (it == m_modules.constEnd())
    {
        sendCallback(id, false, makeError(QStringLiteral("unknown module: ") + moduleName));
        return;
    }

    const std::shared_ptr<IJsApiModule> module = it.value();
    if (m_gate && !m_gate->allows(module->requiredPermissions()))
    {
        sendCallback(id, false, makeError(QStringLiteral("permission denied: ") + method));
        return;
    }

    module->invoke(action, params, [this, id](bool ok, const QJsonValue& payload) {
        sendCallback(id, ok, payload);
    });
}

void MiniAppBridge::sendCallback(qint64 id, bool ok, const QJsonValue& payload)
{
    dispatchToJs(buildCallback(id, ok, payload));
}

void MiniAppBridge::dispatchToJs(const QString& json)
{
    if (!m_webView)
    {
        return;
    }
    // Feed the JSON to the page-side SDK dispatcher (compact JSON is valid JS).
    const QString js = QStringLiteral("if(window.__miniapp){window.__miniapp._dispatch(%1);}").arg(json);
    m_webView->evaluateJavaScript(js);
}

} // namespace MiniAppRuntime
