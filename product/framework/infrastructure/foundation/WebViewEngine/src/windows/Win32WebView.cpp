#include "Win32WebViewImpl.h"

#if defined(_WIN32)

#include <optional>

#include "WebViewSchemeUtils.h"

namespace ucf::infrastructure::webview {

using detail::utf8ToWide;
using detail::wideToUtf8;

// =============================================================================
// Win32WebView — public shell. The heavy lifting lives in the nested Impl,
// split across Win32WebViewLifecycle.cpp (window + async creation + events) and
// Win32WebViewResourceHandler.cpp (WebResourceRequested handling).
// =============================================================================

Win32WebView::Win32WebView()
    : m_impl(std::make_unique<Impl>(this))
{
}

Win32WebView::~Win32WebView()
{
    // Stop async callbacks from touching a half-destroyed object, then tear the
    // WebView2 objects down before the host window.
    m_impl->alive->store(false);
    if (m_impl->controller)
    {
        m_impl->controller->Close();
    }
    m_impl->webView.Reset();
    m_impl->controller.Reset();
    m_impl->environment.Reset();
    m_impl->destroyHostWindow();
}

bool Win32WebView::initialize(const WebViewInitOptions& options)
{
    if (m_impl->initialized)
    {
        return false;
    }

    // Validate, deduplicate, and default the custom schemes (shared with the mac backend).
    std::optional<std::vector<std::string>> schemes =
        scheme_utils::normalizeCustomSchemes(options.customSchemes);
    if (!schemes)
    {
        return false;
    }

    m_impl->initialized = true;
    m_impl->options = options;
    m_impl->customSchemes = *schemes;
    // WebView2 exposes a single message channel (window.chrome.webview); report
    // it under the first configured channel, which the runtime agent uses as its
    // bridge channel.
    m_impl->reportChannel = options.scriptChannels.empty() ? std::string() : options.scriptChannels.front();

    if (!m_impl->createHostWindow())
    {
        return false;
    }
    return m_impl->startEnvironmentCreation();
}

bool Win32WebView::isReady() const
{
    return m_impl->ready.load();
}

void Win32WebView::loadUrl(const std::string& url)
{
    if (!m_impl->webView || url.empty())
    {
        return;
    }
    m_impl->webView->Navigate(utf8ToWide(url).c_str());
}

void Win32WebView::reload()
{
    if (m_impl->webView)
    {
        m_impl->webView->Reload();
    }
}

void Win32WebView::stop()
{
    if (m_impl->webView)
    {
        m_impl->webView->Stop();
    }
}

void Win32WebView::evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback)
{
    if (!m_impl->webView || js.empty())
    {
        if (callback)
        {
            callback(false, "", WebError{ -1, "web view not ready or empty script" });
        }
        return;
    }

    if (!callback)
    {
        m_impl->webView->ExecuteScript(utf8ToWide(js).c_str(), nullptr);
        return;
    }

    auto flag = m_impl->alive;
    m_impl->webView->ExecuteScript(
        utf8ToWide(js).c_str(),
        Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
            [callback, flag](HRESULT result, LPCWSTR resultJson) -> HRESULT {
                if (!flag->load())
                {
                    return S_OK;
                }
                if (FAILED(result))
                {
                    callback(false, "", WebError{ static_cast<int>(result), "ExecuteScript failed" });
                    return S_OK;
                }
                callback(true, wideToUtf8(resultJson), WebError{});
                return S_OK;
            })
            .Get());
}

InterceptorId Win32WebView::addRequestInterceptor(std::shared_ptr<IRequestInterceptor> interceptor)
{
    return m_impl->dispatcher.add(std::move(interceptor));
}

void Win32WebView::removeRequestInterceptor(InterceptorId id)
{
    m_impl->dispatcher.remove(id);
}

void Win32WebView::clearRequestInterceptors()
{
    m_impl->dispatcher.clear();
}

NativeHostHandle Win32WebView::nativeHostHandle() const
{
    return reinterpret_cast<NativeHostHandle>(m_impl->hostWindow);
}

// -----------------------------------------------------------------------------
// Notification forwarders
// -----------------------------------------------------------------------------

void Win32WebView::emitWebViewReady()
{
    fireNotification(&IWebViewCallback::onWebViewReady);
}

void Win32WebView::emitNavigationStarted(const std::string& url)
{
    fireNotification(&IWebViewCallback::onNavigationStarted, url);
}

void Win32WebView::emitUrlChanged(const std::string& url)
{
    fireNotification(&IWebViewCallback::onUrlChanged, url);
}

void Win32WebView::emitTitleChanged(const std::string& title)
{
    fireNotification(&IWebViewCallback::onTitleChanged, title);
}

void Win32WebView::emitLoadFinished(bool ok)
{
    fireNotification(&IWebViewCallback::onLoadFinished, ok);
}

void Win32WebView::emitLoadFailed(int code, const std::string& message)
{
    fireNotification(&IWebViewCallback::onLoadFailed, code, message);
}

void Win32WebView::emitScriptMessage(const std::string& channel, const std::string& payload)
{
    fireNotification(&IWebViewCallback::onScriptMessage, channel, payload);
}

} // namespace ucf::infrastructure::webview

#endif // _WIN32
