#pragma once

#include <memory>
#include <string>

#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>
#include <ucf/Infrastructure/WebViewEngine/IRequestInterceptor.h>
#include <ucf/Infrastructure/WebViewEngine/IWebViewCallback.h>
#include <ucf/Infrastructure/WebViewEngine/WebViewInitOptions.h>
#include <ucf/Infrastructure/WebViewEngine/WebViewTypes.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

namespace ucf::infrastructure::webview {

class Infrastructure_EXPORT IWebView
    : public virtual ucf::utilities::INotificationHelper<IWebViewCallback>
{
public:
    IWebView() = default;
    IWebView(const IWebView&) = delete;
    IWebView(IWebView&&) = delete;
    IWebView& operator=(const IWebView&) = delete;
    IWebView& operator=(IWebView&&) = delete;
    ~IWebView() override = default;

public:
    // Threading contract: all IWebView methods must be called on the main/UI
    // thread. The underlying platform web views (WKWebView on macOS, WebView2 on
    // Windows) require main-thread access, and callbacks are delivered on the
    // main thread as well.
    [[nodiscard]] virtual bool initialize(const WebViewInitOptions& options) = 0;
    [[nodiscard]] virtual bool isReady() const = 0;

    virtual void loadUrl(const std::string& url) = 0;
    virtual void reload() = 0;
    virtual void stop() = 0;
    // A null callback runs the script fire-and-forget; otherwise the JSON-encoded
    // result (or error) is delivered on the main thread.
    virtual void evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback = nullptr) = 0;

    [[nodiscard]] virtual InterceptorId addRequestInterceptor(std::shared_ptr<IRequestInterceptor> interceptor) = 0;
    virtual void removeRequestInterceptor(InterceptorId id) = 0;
    virtual void clearRequestInterceptors() = 0;

    [[nodiscard]] virtual NativeHostHandle nativeHostHandle() const = 0;
};

} // namespace ucf::infrastructure::webview
