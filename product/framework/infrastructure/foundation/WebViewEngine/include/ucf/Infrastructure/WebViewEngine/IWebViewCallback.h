#pragma once

#include <string>

#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>

namespace ucf::infrastructure::webview {

class Infrastructure_EXPORT IWebViewCallback
{
public:
    IWebViewCallback() = default;
    IWebViewCallback(const IWebViewCallback&) = delete;
    IWebViewCallback(IWebViewCallback&&) = delete;
    IWebViewCallback& operator=(const IWebViewCallback&) = delete;
    IWebViewCallback& operator=(IWebViewCallback&&) = delete;
    virtual ~IWebViewCallback() = default;

public:
    // Fired once when the web view has finished (possibly asynchronous)
    // initialization and is safe to drive (loadUrl / evaluateJavaScript).
    // Synchronous backends may deliver this on a later main-loop turn, so
    // register the callback before initialize() or query isReady().
    virtual void onWebViewReady() {}
    virtual void onNavigationStarted(const std::string& /*url*/) {}
    virtual void onUrlChanged(const std::string& /*url*/) {}
    virtual void onTitleChanged(const std::string& /*title*/) {}
    virtual void onLoadFinished(bool /*ok*/) {}
    virtual void onLoadFailed(int /*code*/, const std::string& /*message*/) {}
    virtual void onScriptMessage(const std::string& /*channel*/, const std::string& /*payload*/) {}
};

} // namespace ucf::infrastructure::webview
