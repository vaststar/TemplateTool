#include "StubWebView.h"

namespace ucf::infrastructure::webview {

bool StubWebView::initialize(const WebViewInitOptions& options)
{
    if (m_initialized)
    {
        return false;
    }
    m_initialized = true;
    m_ready = true;
    m_options = options;
    fireNotification(&IWebViewCallback::onWebViewReady);
    return true;
}

bool StubWebView::isReady() const
{
    return m_ready;
}

void StubWebView::loadUrl(const std::string& url)
{
    if (url.empty())
    {
        fireNotification(&IWebViewCallback::onLoadFailed, -1, std::string("url is empty"));
        fireNotification(&IWebViewCallback::onLoadFinished, false);
        return;
    }

    fireNotification(&IWebViewCallback::onNavigationStarted, url);
    fireNotification(&IWebViewCallback::onUrlChanged, url);
    fireNotification(&IWebViewCallback::onLoadFinished, true);
}

void StubWebView::reload()
{
}

void StubWebView::stop()
{
}

void StubWebView::evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback)
{
    (void)js;
    // No JS engine in the stub; report success with a null result so headless
    // consumers can proceed without special-casing the stub backend.
    if (callback)
    {
        callback(true, "null", WebError{});
    }
}

InterceptorId StubWebView::addRequestInterceptor(std::shared_ptr<IRequestInterceptor> interceptor)
{
    return m_dispatcher.add(std::move(interceptor));
}

void StubWebView::removeRequestInterceptor(InterceptorId id)
{
    m_dispatcher.remove(id);
}

void StubWebView::clearRequestInterceptors()
{
    m_dispatcher.clear();
}

NativeHostHandle StubWebView::nativeHostHandle() const
{
    return 0;
}

} // namespace ucf::infrastructure::webview
