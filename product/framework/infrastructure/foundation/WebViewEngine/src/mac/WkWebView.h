#pragma once

#include <memory>

#include "WebViewBase.h"

namespace ucf::infrastructure::webview {

class WkWebView final : public WebViewBase
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

    void loadUrl(const std::string& url) override;
    void reload() override;
    void stop() override;
    void evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback) override;

    [[nodiscard]] NativeHostHandle nativeHostHandle() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ucf::infrastructure::webview
