#pragma once

#include <memory>

#include "MiniAppBridgeCore.h"
#include <ucf/Agents/MiniAppRuntimeAgent/IMiniAppRuntimeAgent.h>
#include <ucf/Infrastructure/WebViewEngine/IWebView.h>
#include <ucf/Infrastructure/WebViewEngine/IWebViewCallback.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::agents {

class RuntimeWebViewCallback;

class MiniAppRuntimeAgent final : public ucf::utilities::NotificationHelper<IMiniAppRuntimeAgentCallback>,
                                  public IMiniAppRuntimeAgent
{
public:
    MiniAppRuntimeAgent();
    ~MiniAppRuntimeAgent() override;

    bool initialize(const MiniAppRuntimeAgentConfig& config) override;
    void shutdown() override;
    [[nodiscard]] bool isReady() const override;

    void loadEntry() override;

    void postBridgeMessage(const std::string& json) override;

    void registerBridgeHandler(std::shared_ptr<IBridgeMethodHandler> handler) override;
    void clearBridgeHandlers() override;

    [[nodiscard]] ucf::infrastructure::webview::NativeHostHandle nativeHostHandle() const override;

private:
    void onWebViewReady();
    void onWebViewUrlChanged(const std::string& url);
    void onWebViewTitleChanged(const std::string& title);
    void onWebViewLoadFinished(bool success);
    void onWebViewLoadFailed(int errorCode, const std::string& errorMessage);
    void onWebViewScriptMessage(const std::string& channel, const std::string& payload);

    // Builds the entry URL from config. Caller must hold m_impl->mutex.
    [[nodiscard]] std::string buildEntryUrlLocked() const;

private:
    friend class RuntimeWebViewCallback;

    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ucf::agents
