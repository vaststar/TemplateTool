#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>

#include <ucf/Agents/MiniAppRuntimeAgent/IBridgeMethodHandler.h>

namespace ucf::agents {

class Infrastructure_EXPORT MiniAppBridgeCore
{
public:
    MiniAppBridgeCore();
    ~MiniAppBridgeCore();

    MiniAppBridgeCore(const MiniAppBridgeCore&) = delete;
    MiniAppBridgeCore(MiniAppBridgeCore&&) = delete;
    MiniAppBridgeCore& operator=(const MiniAppBridgeCore&) = delete;
    MiniAppBridgeCore& operator=(MiniAppBridgeCore&&) = delete;

public:
    // Sink that delivers an outbound JSON message into the page. Set by the
    // owner (the runtime agent); the core stays free of any web-view dependency.
    void setMessageSink(std::function<void(const std::string&)> sink);
    void setGrantedPermissions(const std::vector<std::string>& grantedPermissions);

    void registerHandler(std::shared_ptr<IBridgeMethodHandler> handler);
    void clearHandlers();

    void onInboundMessage(const std::string& json);

    void postEvent(const std::string& eventName, const JsonValue& data);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ucf::agents
