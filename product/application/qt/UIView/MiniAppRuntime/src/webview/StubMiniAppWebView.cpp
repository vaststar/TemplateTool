#include "webview/StubMiniAppWebView.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace MiniAppRuntime {

StubMiniAppWebView::StubMiniAppWebView(QObject* parent)
    : IMiniAppWebView(parent)
{
    UIVIEW_LOG_INFO("StubMiniAppWebView created (no native rendering)");
}

StubMiniAppWebView::~StubMiniAppWebView() = default;

void StubMiniAppWebView::loadUrl(const QUrl& url)
{
    UIVIEW_LOG_INFO("StubMiniAppWebView::loadUrl " << url.toString().toStdString());
    emit loadFinished(false);
}

void StubMiniAppWebView::evaluateJavaScript(const QString& js)
{
    UIVIEW_LOG_DEBUG("StubMiniAppWebView::evaluateJavaScript bytes=" << js.size());
}

QWindow* StubMiniAppWebView::asQWindow()
{
    // Not embeddable: the host window shows a placeholder instead.
    return nullptr;
}

} // namespace MiniAppRuntime
