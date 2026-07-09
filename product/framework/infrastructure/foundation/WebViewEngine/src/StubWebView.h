#pragma once

#include <ucf/Infrastructure/WebViewEngine/IWebView.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include "InterceptorDispatcher.h"

namespace ucf::infrastructure::webview {

class StubWebView final
    : public virtual IWebView
    , public virtual ucf::utilities::NotificationHelper<IWebViewCallback>
{
public:
    StubWebView() = default;
    StubWebView(const StubWebView&) = delete;
    StubWebView(StubWebView&&) = delete;
    StubWebView& operator=(const StubWebView&) = delete;
    StubWebView& operator=(StubWebView&&) = delete;

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

private:
    bool m_initialized = false;
    bool m_ready = false;
    WebViewInitOptions m_options;
    InterceptorDispatcher m_dispatcher;
};

} // namespace ucf::infrastructure::webview
