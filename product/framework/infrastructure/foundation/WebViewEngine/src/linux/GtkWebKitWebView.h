#pragma once

#include <memory>

#include "WebViewBase.h"

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
class GtkWebKitWebView final : public WebViewBase
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

    void loadUrl(const std::string& url) override;
    void reload() override;
    void stop() override;
    void evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback) override;

    [[nodiscard]] NativeHostHandle nativeHostHandle() const override;

public:
    struct Impl;
private:
    std::unique_ptr<Impl> m_impl;
};

} // namespace ucf::infrastructure::webview
