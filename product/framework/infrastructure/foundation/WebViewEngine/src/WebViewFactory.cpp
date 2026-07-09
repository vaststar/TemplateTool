#include <ucf/Infrastructure/WebViewEngine/WebViewFactory.h>

#include "StubWebView.h"

#if defined(__APPLE__)
#include "WkWebView.h"
#elif defined(_WIN32)
#include "Win32WebView.h"
#endif

namespace ucf::infrastructure::webview {

std::shared_ptr<IWebView> createWebView()
{
#if defined(__APPLE__)
    return std::make_shared<WkWebView>();
#elif defined(_WIN32)
    return std::make_shared<Win32WebView>();
#else
    return std::make_shared<StubWebView>();
#endif
}

} // namespace ucf::infrastructure::webview
