#include "webview/MiniAppWebViewFactory.h"

#include <QtGlobal>

#if defined(Q_OS_MACOS)
#include "webview/WkWebViewBackend.h"
#else
#include "webview/StubMiniAppWebView.h"
#endif

namespace MiniAppRuntime {

IMiniAppWebView* createMiniAppWebView(QObject* parent)
{
#if defined(Q_OS_MACOS)
    return new WkWebViewBackend(parent);
#else
    // TODO: Windows -> WebView2 backend. Until then, the portable stub.
    return new StubMiniAppWebView(parent);
#endif
}

} // namespace MiniAppRuntime
