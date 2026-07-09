#include "MiniAppBridge.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QVariant>

#include "PermissionGate.h"
#include "jsapi/IJsApiModule.h"
#include "webview/IMiniAppWebView.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace MiniAppRuntime {

namespace {

ucf::agents::JsonValue toAgentJson(const QJsonValue& value)
{
    if (value.isUndefined() || value.isNull())
    {
        return ucf::agents::JsonValue();
    }
    if (value.isBool())
    {
        return ucf::agents::JsonValue(value.toBool());
    }
    if (value.isDouble())
    {
        return ucf::agents::JsonValue(value.toDouble());
    }
    if (value.isString())
    {
        return ucf::agents::JsonValue(value.toString().toStdString());
    }
    if (value.isArray())
    {
        std::vector<ucf::agents::JsonValue> out;
        const QJsonArray array = value.toArray();
        out.reserve(static_cast<size_t>(array.size()));
        for (const auto& it : array)
        {
            out.push_back(toAgentJson(it));
        }
        return ucf::agents::JsonValue::array(std::move(out));
    }
    if (value.isObject())
    {
        std::map<std::string, ucf::agents::JsonValue> out;
        const QJsonObject object = value.toObject();
        for (auto it = object.constBegin(); it != object.constEnd(); ++it)
        {
            out[it.key().toStdString()] = toAgentJson(it.value());
        }
        return ucf::agents::JsonValue::object(std::move(out));
    }
    return ucf::agents::JsonValue();
}

QJsonValue toQtJson(const ucf::agents::JsonValue& value)
{
    if (value.isNull())
    {
        return QJsonValue();
    }
    if (const auto boolVal = value.asBool(); boolVal.has_value())
    {
        return QJsonValue(*boolVal);
    }
    if (value.isNumber())
    {
        return QJsonValue(value.asDouble().value_or(0.0));
    }
    if (const auto stringVal = value.asString(); stringVal.has_value())
    {
        return QJsonValue(QString::fromStdString(*stringVal));
    }
    if (value.isArray())
    {
        QJsonArray array;
        for (const auto& child : value.toArray())
        {
            array.push_back(toQtJson(child));
        }
        return array;
    }
    if (value.isObject())
    {
        QJsonObject object;
        const auto keys = value.keys();
        for (const auto& key : keys)
        {
            object.insert(QString::fromStdString(key), toQtJson(value.get(key)));
        }
        return object;
    }
    return QJsonValue();
}

class ModuleHandlerAdapter : public ucf::agents::IBridgeMethodHandler
{
public:
    explicit ModuleHandlerAdapter(std::shared_ptr<IJsApiModule> module)
        : m_module(std::move(module))
    {
    }

    [[nodiscard]] std::string moduleName() const override
    {
        return m_module ? m_module->name().toStdString() : "";
    }

    [[nodiscard]] std::vector<std::string> requiredPermissions() const override
    {
        if (!m_module)
        {
            return {};
        }

        const QStringList permissions = m_module->requiredPermissions();
        std::vector<std::string> out;
        out.reserve(static_cast<size_t>(permissions.size()));
        for (const auto& permission : permissions)
        {
            out.push_back(permission.toStdString());
        }
        return out;
    }

    void handle(const std::string& action, const ucf::agents::JsonValue& params, Reply reply) override
    {
        if (!m_module)
        {
            reply(false, ucf::agents::JsonValue::object());
            return;
        }

        QJsonObject paramsObj;
        const QJsonValue qtParams = toQtJson(params);
        if (qtParams.isObject())
        {
            paramsObj = qtParams.toObject();
        }

        m_module->invoke(QString::fromStdString(action),
                         paramsObj,
                         [reply = std::move(reply)](bool ok, const QJsonValue& payload) {
                             reply(ok, toAgentJson(payload));
                         });
    }

private:
    std::shared_ptr<IJsApiModule> m_module;
};

} // namespace

class QtBridgeTransportAdapter : public ucf::agents::IBridgeTransport
{
public:
    void setWebView(IMiniAppWebView* webView)
    {
        m_webView = webView;
    }

    void postToJs(const std::string& json) override
    {
        if (!m_webView)
        {
            return;
        }

        const QString js = QStringLiteral("if(window.__miniapp){window.__miniapp._dispatch(%1);}")
                               .arg(QString::fromStdString(json));
        m_webView->evaluateJavaScript(js);
    }

private:
    IMiniAppWebView* m_webView = nullptr; // not owned
};

MiniAppBridge::MiniAppBridge(QObject* parent)
    : QObject(parent)
    , m_transport(std::make_unique<QtBridgeTransportAdapter>())
{
    m_core.setTransport(m_transport.get());
}

MiniAppBridge::~MiniAppBridge() = default;

void MiniAppBridge::setPermissionGate(PermissionGate* gate)
{
    m_gate = gate;

    std::vector<std::string> granted;
    if (m_gate)
    {
        const QStringList tokens = m_gate->granted();
        granted.reserve(static_cast<size_t>(tokens.size()));
        for (const QString& token : tokens)
        {
            granted.push_back(token.toStdString());
        }
    }
    m_core.setGrantedPermissions(granted);
}

void MiniAppBridge::registerModule(std::unique_ptr<IJsApiModule> module)
{
    if (!module)
    {
        return;
    }
    auto moduleRef = std::shared_ptr<IJsApiModule>(std::move(module));
    auto adapter = std::make_shared<ModuleHandlerAdapter>(std::move(moduleRef));
    m_core.registerHandler(adapter);
    m_handlerAdapters.push_back(std::move(adapter));
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
    m_transport->setWebView(m_webView);
    if (m_webView)
    {
        connect(m_webView, &IMiniAppWebView::messageReceived, this, &MiniAppBridge::onMessageReceived);
    }
}

void MiniAppBridge::emitEvent(const QString& event, const QJsonValue& data)
{
    m_core.emitEvent(event.toStdString(), toAgentJson(data));
}

void MiniAppBridge::onMessageReceived(const QString& json)
{
    m_core.onInboundMessage(json.toStdString());
}

} // namespace MiniAppRuntime
