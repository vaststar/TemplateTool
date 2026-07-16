#include "StubWebView.h"

namespace ucf::infrastructure::webview {

bool StubWebView::initialize(const WebViewInitOptions& options)
{
    if (isInitialized())
    {
        return false;
    }
    markInitialized();
    m_options = options;
    markReady();
    return true;
}

void StubWebView::loadUrl(const std::string& url)
{
    if (url.empty())
    {
        emitLoadFailed(-1, std::string("url is empty"));
        emitLoadFinished(false);
        return;
    }

    emitNavigationStarted(url);
    emitUrlChanged(url);
    emitLoadFinished(true);
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

NativeHostHandle StubWebView::nativeHostHandle() const
{
    return 0;
}

} // namespace ucf::infrastructure::webview
