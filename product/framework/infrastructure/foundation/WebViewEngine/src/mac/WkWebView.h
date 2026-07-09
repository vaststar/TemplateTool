#pragma once

#include <memory>

#include <ucf/Infrastructure/WebViewEngine/IWebView.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include "InterceptorDispatcher.h"

namespace ucf::infrastructure::webview {

class WkWebView final
    : public virtual IWebView
    , public virtual ucf::utilities::NotificationHelper<IWebViewCallback>
{
public:
    WkWebView();
    WkWebView(const WkWebView&) = delete;
    WkWebView(WkWebView&&) = delete;
    WkWebView& operator=(const WkWebView&) = delete;
    WkWebView& operator=(WkWebView&&) = delete;
    ~WkWebView() override;

public:
    [[nodiscard]] bool initialize(const WebViewInitOptions& options) override;
    [[nodiscard]] bool isReady() const override;

    void loadUrl(const std::string& url) override;
    void reload() override;
    void stop() override;
    void evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback) override;

    [[nodiscard]] InterceptorId addRequestInterceptor(std::shared_ptr<IRequestInterceptor> interceptor) override;
    void removeRequestInterceptor(InterceptorId id) override;
    void clearRequestInterceptors() override;

    [[nodiscard]] NativeHostHandle nativeHostHandle() const override;

    // Platform callback dispatch methods (internal use by platform backends)
    void emitWebViewReady()
    {
        fireNotification(&IWebViewCallback::onWebViewReady);
    }

    void emitNavigationStarted(const std::string& url)
    {
        fireNotification(&IWebViewCallback::onNavigationStarted, url);
    }

    void emitUrlChanged(const std::string& url)
    {
        fireNotification(&IWebViewCallback::onUrlChanged, url);
    }

    void emitTitleChanged(const std::string& title)
    {
        fireNotification(&IWebViewCallback::onTitleChanged, title);
    }

    void emitLoadFinished(bool ok)
    {
        fireNotification(&IWebViewCallback::onLoadFinished, ok);
    }

    void emitLoadFailed(int code, const std::string& message)
    {
        fireNotification(&IWebViewCallback::onLoadFailed, code, message);
    }

    void emitScriptMessage(const std::string& channel, const std::string& payload)
    {
        fireNotification(&IWebViewCallback::onScriptMessage, channel, payload);
    }

    // Internal: expose dispatcher for platform backends (used from Objective-C bridge)
    InterceptorDispatcher& internalDispatcher();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ucf::infrastructure::webview
