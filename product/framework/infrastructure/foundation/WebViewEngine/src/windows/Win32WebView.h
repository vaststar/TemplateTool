#pragma once

#include <memory>

#include "WebViewBase.h"

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
class Win32WebView final : public WebViewBase
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

    void loadUrl(const std::string& url) override;
    void reload() override;
    void stop() override;
    void evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback) override;

    [[nodiscard]] NativeHostHandle nativeHostHandle() const override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

    // Off-thread resource resolution marshals results through this struct, which
    // needs to name the otherwise-private Impl.
    friend struct PendingResourceResponse;
};

} // namespace ucf::infrastructure::webview
