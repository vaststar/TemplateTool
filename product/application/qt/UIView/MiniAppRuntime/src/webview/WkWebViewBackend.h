#pragma once

#if defined(Q_OS_MACOS) || defined(__APPLE__)

#include "webview/IMiniAppWebView.h"

#include <memory>

namespace MiniAppRuntime {

// macOS WKWebView backend. All Objective-C/WebKit details live in the .mm; this
// header stays pure C++ (Pimpl) so the plain-C++ factory can include it.
class WkWebViewBackend : public IMiniAppWebView
{
    Q_OBJECT
public:
    explicit WkWebViewBackend(QObject* parent = nullptr);
    ~WkWebViewBackend() override;

    void loadUrl(const QUrl& url) override;
    void evaluateJavaScript(const QString& js) override;
    QWindow* asQWindow() override;

    // Called by the Objective-C helpers (in the .mm) to surface events on the
    // C++ side. Not part of the public interface.
    void notifyMessage(const QString& body);
    void notifyLoadFinished(bool ok);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace MiniAppRuntime

#endif // Q_OS_MACOS
