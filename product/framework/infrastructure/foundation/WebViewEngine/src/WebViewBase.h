#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <ucf/Infrastructure/WebViewEngine/IWebView.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include "InterceptorDispatcher.h"

namespace ucf::infrastructure::webview {

// Internal (src-only) base shared by every platform backend. It sits between the
// public IWebView contract and each concrete backend, owning the pieces that
// were previously duplicated verbatim in every backend:
//   * the NotificationHelper<IWebViewCallback> wiring
//   * the emit* notification forwarders (called from platform callbacks)
//   * the InterceptorDispatcher plus the add/remove/clear plumbing
//
// Platform backends inherit this instead of (IWebView + NotificationHelper) and
// only implement the engine-specific pieces (initialize / loadUrl / ...).
//
// NOT part of the public include tree; lives under src/ intentionally. The
// emit* forwarders and internalDispatcher() are public because free-function
// engine callbacks (WebView2 handlers, GTK signal trampolines, WKWebView
// delegates) invoke them through a backend pointer.
class WebViewBase
    : public virtual IWebView
    , public virtual ucf::utilities::NotificationHelper<IWebViewCallback>
{
public:
    WebViewBase() = default;
    ~WebViewBase() override = default;

    WebViewBase(const WebViewBase&) = delete;
    WebViewBase(WebViewBase&&) = delete;
    WebViewBase& operator=(const WebViewBase&) = delete;
    WebViewBase& operator=(WebViewBase&&) = delete;

    // --- IWebView: request interception ---------------------------------------
    // Engine-agnostic forwarding to the shared dispatcher. Marked final so the
    // behaviour cannot diverge per backend again.
    [[nodiscard]] InterceptorId addRequestInterceptor(
        std::shared_ptr<IRequestInterceptor> interceptor) final;
    void removeRequestInterceptor(InterceptorId id) final;
    void clearRequestInterceptors() final;

    // Dispatcher access for the platform resource handlers that run the
    // intercept loop (WebResourceRequested / uri-scheme / WKURLScheme handlers).
    [[nodiscard]] InterceptorDispatcher& internalDispatcher() noexcept { return m_dispatcher; }
    [[nodiscard]] const InterceptorDispatcher& internalDispatcher() const noexcept { return m_dispatcher; }

    // Shared readiness flag. Backends flip it via markReady()/setReady(); the
    // agent polls isReady() before driving loadUrl / evaluateJavaScript.
    [[nodiscard]] bool isReady() const final { return m_ready.load(); }

    // Idempotent "set-once + emit": flips the ready flag and fires
    // onWebViewReady exactly once. Used by backends whose readiness and
    // notification happen together (WebView2 controller ready, content filter
    // installed).
    void markReady()
    {
        if (!m_ready.exchange(true))
        {
            emitWebViewReady();
        }
    }

    // Flip the ready flag without emitting. For backends that must expose
    // readiness synchronously but deliver onWebViewReady on a later main-loop
    // turn (they call emitWebViewReady() themselves once scheduled).
    void setReady(bool ready = true) noexcept { m_ready.store(ready); }

    // --- Notification forwarders (previously copied into every backend) --------
    void emitWebViewReady() { fireNotification(&IWebViewCallback::onWebViewReady); }
    void emitNavigationStarted(const std::string& url) { fireNotification(&IWebViewCallback::onNavigationStarted, url); }
    void emitUrlChanged(const std::string& url) { fireNotification(&IWebViewCallback::onUrlChanged, url); }
    void emitTitleChanged(const std::string& title) { fireNotification(&IWebViewCallback::onTitleChanged, title); }
    void emitLoadFinished(bool ok) { fireNotification(&IWebViewCallback::onLoadFinished, ok); }
    void emitLoadFailed(int code, const std::string& message) { fireNotification(&IWebViewCallback::onLoadFailed, code, message); }
    void emitScriptMessage(const std::string& channel, const std::string& payload) { fireNotification(&IWebViewCallback::onScriptMessage, channel, payload); }

protected:
    // Re-entrancy guard for initialize(): only ever touched from a backend's own
    // initialize() on the UI thread.
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }
    void markInitialized() noexcept { m_initialized = true; }

private:
    bool m_initialized = false;
    std::atomic<bool> m_ready { false };
    InterceptorDispatcher m_dispatcher;
};

} // namespace ucf::infrastructure::webview
