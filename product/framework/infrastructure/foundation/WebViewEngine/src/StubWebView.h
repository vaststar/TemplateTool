#pragma once

#include "WebViewBase.h"

namespace ucf::infrastructure::webview {

class StubWebView final : public WebViewBase
{
public:
    StubWebView() = default;
    StubWebView(const StubWebView&) = delete;
    StubWebView(StubWebView&&) = delete;
    StubWebView& operator=(const StubWebView&) = delete;
    StubWebView& operator=(StubWebView&&) = delete;

public:
    [[nodiscard]] bool initialize(const WebViewInitOptions& options) override;

    void loadUrl(const std::string& url) override;
    void reload() override;
    void stop() override;
    void evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback) override;

    [[nodiscard]] NativeHostHandle nativeHostHandle() const override;

private:
    WebViewInitOptions m_options;
};

} // namespace ucf::infrastructure::webview
