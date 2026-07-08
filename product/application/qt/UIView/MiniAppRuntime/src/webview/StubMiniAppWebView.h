#pragma once

#include "webview/IMiniAppWebView.h"

namespace MiniAppRuntime {

// Portable placeholder backend for platforms without a native web view. Renders
// nothing and cannot be embedded; only logs calls so the rest of the runtime works.
class StubMiniAppWebView : public IMiniAppWebView
{
    Q_OBJECT
public:
    explicit StubMiniAppWebView(QObject* parent = nullptr);
    ~StubMiniAppWebView() override;

    void loadUrl(const QUrl& url) override;
    void evaluateJavaScript(const QString& js) override;
    QWindow* asQWindow() override;
};

} // namespace MiniAppRuntime
