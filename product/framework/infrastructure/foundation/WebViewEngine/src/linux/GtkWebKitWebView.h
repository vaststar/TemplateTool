#pragma once

#include <memory>

#include <ucf/Infrastructure/WebViewEngine/IWebView.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::infrastructure::webview {

// Linux backend built on WebKitGTK (webkit2gtk-4.1) + GTK 3 on X11.
//
// WebKitGTK is the natural mirror of the macOS WKWebView backend: same WebKit
// engine, system-provided runtime, and an embeddable GtkWidget. A few Linux
// specifics shape this design:
//   1. Embedding relies on X11 window reparenting. The backend owns an internal
//      top-level GtkWindow that hosts the WebKitWebView and returns that
//      window's X11 XID from nativeHostHandle(); the UI embeds it via
//      QWindow::fromWinId + QWidget::createWindowContainer. This works under
//      X11 and XWayland, but NOT under native Wayland (no foreign-window
//      embedding) -- the app must run with QT_QPA_PLATFORM=xcb.
//   2. GTK/WebKit signals and async callbacks are delivered on the GLib main
//      context, which Qt already pumps on the main thread (QEventDispatcherGlib),
//      so callbacks arrive on the UI thread as required by the IWebView contract.
//   3. The declarative network policy reuses the same JSON rule format as the
//      macOS WKContentRuleList, compiled here via WebKitUserContentFilter. When
//      a policy is present readiness is deferred until the filter is installed
//      (fail-closed), matching the macOS behaviour.
//
// All methods must be called on the UI (main) thread.
class GtkWebKitWebView final
    : public virtual IWebView
    , public virtual ucf::utilities::NotificationHelper<IWebViewCallback>
{
public:
    GtkWebKitWebView();
    GtkWebKitWebView(const GtkWebKitWebView&) = delete;
    GtkWebKitWebView(GtkWebKitWebView&&) = delete;
    GtkWebKitWebView& operator=(const GtkWebKitWebView&) = delete;
    GtkWebKitWebView& operator=(GtkWebKitWebView&&) = delete;
    ~GtkWebKitWebView() override;

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

    // Notification forwarders invoked by the GTK/WebKit signal handlers (defined
    // in the .cpp). Public so the free-function C trampolines can reach them.
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

    // Internal: expose the dispatcher to the custom-scheme trampoline.
    class InterceptorDispatcher& internalDispatcher();

public:
    struct Impl;
private:
    std::unique_ptr<Impl> m_impl;
};

} // namespace ucf::infrastructure::webview
