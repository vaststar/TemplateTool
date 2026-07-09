#pragma once

#include <memory>
#include <string>

#include <ucf/Agents/MiniAppRuntimeAgent/IBridgeMethodHandler.h>
#include <ucf/Agents/MiniAppRuntimeAgent/IMiniAppRuntimeAgentCallback.h>
#include <ucf/Agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentConfig.h>
#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>
#include <ucf/Infrastructure/WebViewEngine/WebViewTypes.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

namespace ucf::agents {

class Infrastructure_EXPORT IMiniAppRuntimeAgent
    : public virtual ucf::utilities::INotificationHelper<IMiniAppRuntimeAgentCallback>
{
public:
    ~IMiniAppRuntimeAgent() override = default;

    virtual bool initialize(const MiniAppRuntimeAgentConfig& config) = 0;
    virtual void shutdown() = 0;
    [[nodiscard]] virtual bool isReady() const = 0;

    virtual void loadEntry() = 0;

    // Send a JSON payload to page-side bridge dispatcher.
    virtual void postBridgeMessage(const std::string& json) = 0;

    virtual void registerBridgeHandler(std::shared_ptr<IBridgeMethodHandler> handler) = 0;
    virtual void clearBridgeHandlers() = 0;

    [[nodiscard]] virtual ucf::infrastructure::webview::NativeHostHandle nativeHostHandle() const = 0;
};

} // namespace ucf::agents
