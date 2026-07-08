#include "MiniAppSession.h"

#include <QUrl>

#include "AppSchemeHandler.h"
#include "MiniAppBridge.h"
#include "MiniAppSdk.h"
#include "PermissionGate.h"
#include "jsapi/SystemApiModule.h"
#include "webview/IMiniAppWebView.h"
#include "webview/MiniAppWebViewFactory.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace MiniAppRuntime {

namespace {
constexpr const char* kAppScheme = "app";
}

MiniAppSession::MiniAppSession(MiniAppContext context, QObject* parent)
    : QObject(parent)
    , m_context(std::move(context))
    , m_webView(createMiniAppWebView(this))
    , m_schemeHandler(std::make_unique<AppSchemeHandler>(m_context.packageDir))
    , m_permissionGate(std::make_unique<PermissionGate>())
    , m_bridge(std::make_unique<MiniAppBridge>(this))
{
}

MiniAppSession::~MiniAppSession() = default;

IMiniAppWebView* MiniAppSession::webView() const
{
    return m_webView;
}

void MiniAppSession::start()
{
    if (m_started)
    {
        return;
    }
    m_started = true;

    m_permissionGate->setGranted(m_context.permissions);

    // Wire the bridge: permissions, built-in modules, and the message channel.
    m_bridge->setPermissionGate(m_permissionGate.get());
    m_bridge->registerModule(std::make_unique<SystemApiModule>(m_context.id, m_context.name));
    m_bridge->attach(m_webView);

    // Prepare the web view before navigation: scheme + injected SDK.
    m_webView->registerScheme(QString::fromLatin1(kAppScheme), m_schemeHandler.get());
    m_webView->addUserScriptAtDocumentStart(QString::fromUtf8(miniAppSdkJs()));

    const QString entry = m_context.entry.isEmpty() ? QStringLiteral("index.html") : m_context.entry;
    const QUrl url(QStringLiteral("%1://%2/%3")
                       .arg(QString::fromLatin1(kAppScheme), m_context.id, entry));
    UIVIEW_LOG_INFO("MiniAppSession start id=" << m_context.id.toStdString()
                                               << " url=" << url.toString().toStdString());
    m_webView->loadUrl(url);
}

} // namespace MiniAppRuntime
