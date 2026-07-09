#pragma once

#include <memory>

#include <ucf/Infrastructure/WebViewEngine/IWebView.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::infrastructure::webview {

// Windows backend built on the Microsoft Edge WebView2 runtime.
//
// WebView2 differs from WKWebView in three ways that shape this design:
//   1. It requires a parent HWND at controller-creation time, so the backend
//      owns an internal host window and returns its HWND from
//      nativeHostHandle() (the UI embeds it via QWindow::fromWinId).
//   2. Environment + controller creation are fully asynchronous, so isReady()
//      stays false until both complete; onWebViewReady() fires afterwards.
//   3. It needs a writable user-data folder (WebViewInitOptions::userDataFolder).
//
// All methods must be called on the UI thread that owns the host window's
// message loop (the IWebView threading contract).
class Win32WebView final
    : public virtual IWebView
    , public virtual ucf::utilities::NotificationHelper<IWebViewCallback>
{
public:
    Win32WebView();
    Win32WebView(const Win32WebView&) = delete;
    Win32WebView(Win32WebView&&) = delete;
    Win32WebView& operator=(const Win32WebView&) = delete;
    Win32WebView& operator=(Win32WebView&&) = delete;
    ~Win32WebView() override;

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
    // Notification forwarders invoked by the WebView2 event handlers (defined in
    // the .cpp). Kept private; the nested Impl accesses them directly.
    void emitWebViewReady();
    void emitNavigationStarted(const std::string& url);
    void emitUrlChanged(const std::string& url);
    void emitTitleChanged(const std::string& title);
    void emitLoadFinished(bool ok);
    void emitLoadFailed(int code, const std::string& message);
    void emitScriptMessage(const std::string& channel, const std::string& payload);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ucf::infrastructure::webview
