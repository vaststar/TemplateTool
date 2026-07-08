#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

class QWindow;

namespace MiniAppRuntime {

class AppSchemeHandler;

// Backend-agnostic surface for a native web view that hosts a mini-app. The
// runtime talks only to this interface, so adding a platform (WKWebView,
// WebView2, stub) never touches the bridge/session. Config shared across
// backends (custom scheme + resolver, document-start scripts) lives here.
class IMiniAppWebView : public QObject
{
    Q_OBJECT
public:
    explicit IMiniAppWebView(QObject* parent = nullptr) : QObject(parent) {}
    ~IMiniAppWebView() override = default;

    // Route the given custom scheme (e.g. "app") through `handler`. Call before loadUrl().
    void registerScheme(const QString& scheme, AppSchemeHandler* handler)
    {
        m_scheme = scheme;
        m_schemeHandler = handler;
        onSchemeRegistered();
    }

    // Inject `js` at document-start on every navigation. Call before loadUrl().
    void addUserScriptAtDocumentStart(const QString& js) { m_pendingScripts.append(js); }

    virtual void loadUrl(const QUrl& url) = 0;

    // Execute `js` in the page's main world (fire-and-forget).
    virtual void evaluateJavaScript(const QString& js) = 0;

    // Embeddable native window, or nullptr if this backend cannot be embedded (stub).
    virtual QWindow* asQWindow() = 0;

    [[nodiscard]] AppSchemeHandler* schemeHandler() const { return m_schemeHandler; }

signals:
    // Raw JSON string posted by the page's SDK through the native channel.
    void messageReceived(const QString& json);
    void loadFinished(bool ok);

protected:
    // Called when registerScheme() runs; backends with an already-created view
    // apply the scheme here. Lazy backends read scheme() at creation and ignore it.
    virtual void onSchemeRegistered() {}

    [[nodiscard]] const QString& scheme() const { return m_scheme; }
    [[nodiscard]] const QStringList& pendingScripts() const { return m_pendingScripts; }

private:
    // `m_schemeHandler` is not owned.
    QString m_scheme;
    AppSchemeHandler* m_schemeHandler = nullptr;
    QStringList m_pendingScripts;
};

} // namespace MiniAppRuntime
