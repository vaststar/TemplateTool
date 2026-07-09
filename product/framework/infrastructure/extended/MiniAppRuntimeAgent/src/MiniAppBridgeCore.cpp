#include "MiniAppBridgeCore.h"

#include <map>
#include <mutex>
#include <unordered_set>
#include <utility>

namespace ucf::agents {
namespace {

JsonValue makeErrorPayload(const std::string& code, const std::string& message)
{
    auto obj = JsonValue::object();
    obj.set("code", code);
    obj.set("message", message);
    return obj;
}

std::string toJsonCallback(std::int64_t id, bool ok, const JsonValue& payload)
{
    auto callback = JsonValue::object();
    callback.set("type", "callback");
    callback.set("id", id);
    callback.set("ok", ok);
    callback.set(ok ? "result" : "error", payload);
    return callback.dump();
}

std::string toJsonEvent(const std::string& eventName, const JsonValue& data)
{
    auto event = JsonValue::object();
    event.set("type", "event");
    event.set("event", eventName);
    event.set("data", data);
    return event.dump();
}

} // namespace

class MiniAppBridgeCore::Impl
{
public:
    std::mutex mutex;
    std::function<void(const std::string&)> sink;
    std::unordered_set<std::string> grantedPermissions;
    std::map<std::string, std::shared_ptr<IBridgeMethodHandler>> handlers;

    void post(const std::string& json)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (sink)
        {
            sink(json);
        }
    }
};

MiniAppBridgeCore::MiniAppBridgeCore()
    : m_impl(std::make_unique<Impl>())
{
}

MiniAppBridgeCore::~MiniAppBridgeCore() = default;

void MiniAppBridgeCore::setMessageSink(std::function<void(const std::string&)> sink)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->sink = std::move(sink);
}

void MiniAppBridgeCore::setGrantedPermissions(const std::vector<std::string>& grantedPermissions)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->grantedPermissions.clear();
    for (const auto& token : grantedPermissions)
    {
        m_impl->grantedPermissions.insert(token);
    }
}

void MiniAppBridgeCore::registerHandler(std::shared_ptr<IBridgeMethodHandler> handler)
{
    if (!handler)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->handlers[handler->moduleName()] = std::move(handler);
}

void MiniAppBridgeCore::unregisterHandler(const std::string& moduleName)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->handlers.erase(moduleName);
}

void MiniAppBridgeCore::clearHandlers()
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->handlers.clear();
}

void MiniAppBridgeCore::onInboundMessage(const std::string& json)
{
    const JsonValue::ParseResult parseResult = JsonValue::parseEx(json);
    if (!parseResult.ok() || !parseResult.value.isObject())
    {
        return;
    }

    const JsonValue root = parseResult.value;
    if (root.get("type").asString().value_or("") != "invoke")
    {
        return;
    }

    const std::int64_t id = root.get("id").asInt64().value_or(-1);
    const std::string method = root.get("method").asString().value_or("");
    JsonValue params = root.contains("params") ? root.get("params") : JsonValue::object();
    if (!params.isObject())
    {
        params = JsonValue::object();
    }

    if (id < 0 || method.empty())
    {
        m_impl->post(toJsonCallback(id, false, makeErrorPayload("invalid_request", "missing id or method")));
        return;
    }

    const auto dotPos = method.find('.');
    if (dotPos == std::string::npos || dotPos == 0 || dotPos == method.size() - 1)
    {
        m_impl->post(toJsonCallback(id, false, makeErrorPayload("invalid_method", "method must be <module>.<action>")));
        return;
    }

    const std::string module = method.substr(0, dotPos);
    const std::string action = method.substr(dotPos + 1);

    std::shared_ptr<IBridgeMethodHandler> handler;
    std::unordered_set<std::string> grantedPermissions;
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        const auto it = m_impl->handlers.find(module);
        if (it != m_impl->handlers.end())
        {
            handler = it->second;
        }
        grantedPermissions = m_impl->grantedPermissions;
    }

    if (!handler)
    {
        m_impl->post(toJsonCallback(id, false, makeErrorPayload("unknown_module", "unknown module: " + module)));
        return;
    }

    for (const auto& required : handler->requiredPermissions())
    {
        if (grantedPermissions.find(required) == grantedPermissions.end())
        {
            m_impl->post(toJsonCallback(id, false, makeErrorPayload("permission_denied", "permission denied: " + method)));
            return;
        }
    }

    handler->handle(action,
                    params,
                    [this, id](bool ok, const JsonValue& payload) {
                        m_impl->post(toJsonCallback(id, ok, payload));
                    });
}

// Currently unused convenience: a typed wrapper that guarantees a well-formed
// {type:"event", ...} envelope. Equivalent output can be produced today via
// MiniAppRuntimeAgent::postBridgeMessage(); this is not wired to the public API.
void MiniAppBridgeCore::emitEvent(const std::string& eventName, const JsonValue& data)
{
    m_impl->post(toJsonEvent(eventName, data));
}

} // namespace ucf::agents
