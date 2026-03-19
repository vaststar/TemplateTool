#include "PageViews/ToolsPage/network/include/NetworkProxyController.h"
#include "PageViews/ToolsPage/network/include/ProxyRequestModel.h"
#include "PageViews/ToolsPage/network/include/ProxyRulesManager.h"
#include "PageViews/ToolsPage/network/include/ProxyCertManager.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/NetworkProxyViewModel/INetworkProxyViewModel.h>

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QUrl>
#include <QUrlQuery>

NetworkProxyController::NetworkProxyController(QObject* parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIVMSignalEmitter::NetworkProxyViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create NetworkProxyController, this=" << (void*)this);
    m_requestModel = new ProxyRequestModel(this);
    m_rulesManager = new ProxyRulesManager(this);
    m_certManager  = new ProxyCertManager(this);

    // Wire cert manager status messages to our status
    connect(m_certManager, &ProxyCertManager::statusMessage,
            this, &NetworkProxyController::setStatusMessage);
}

NetworkProxyController::~NetworkProxyController()
{
    stopProxy();
    UIVIEW_LOG_DEBUG("destroy NetworkProxyController");
}

// ====================== init ======================

void NetworkProxyController::init()
{
    UIVIEW_LOG_DEBUG("NetworkProxyController::init");

    // Connect emitter signals to controller slots
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::NetworkProxyViewModelEmitter::signals_onProxyStateChanged,
            this, &NetworkProxyController::onProxyStateChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::NetworkProxyViewModelEmitter::signals_onAddonConnectionChanged,
            this, &NetworkProxyController::onAddonConnectionChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::NetworkProxyViewModelEmitter::signals_onRequestCaptured,
            this, &NetworkProxyController::onRequestCaptured);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::NetworkProxyViewModelEmitter::signals_onResponseCaptured,
            this, &NetworkProxyController::onResponseCaptured);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::NetworkProxyViewModelEmitter::signals_onRequestIntercepted,
            this, &NetworkProxyController::onRequestIntercepted);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::NetworkProxyViewModelEmitter::signals_onStatusMessage,
            this, &NetworkProxyController::onStatusMessage);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::NetworkProxyViewModelEmitter::signals_onCertStatusChanged,
            this, &NetworkProxyController::onCertStatusChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::NetworkProxyViewModelEmitter::signals_onError,
            this, &NetworkProxyController::onError);

    // Create ViewModel via factory, init, and register emitter as callback
    m_viewModel = getAppContext()->getViewModelFactory()->createNetworkProxyViewModelInstance();
    m_viewModel->initViewModel();
    m_viewModel->registerCallback(m_viewModelEmitter);

    // Wire rules manager — serialize QJsonObject to string and send via ViewModel
    m_rulesManager->setSendCommandFn([this](const QJsonObject& cmd) {
        if (m_viewModel) {
            QJsonDocument doc(cmd);
            m_viewModel->sendCommand(doc.toJson(QJsonDocument::Compact).toStdString());
        }
    });
}

// ====================== Property getters ======================

bool    NetworkProxyController::isProxyRunning()     const { return m_proxyRunning; }
bool    NetworkProxyController::isAddonConnected()   const { return m_addonConnected; }
int     NetworkProxyController::getProxyPort()       const { return m_proxyPort; }
bool    NetworkProxyController::getAutoSystemProxy() const { return m_autoSystemProxy; }
bool    NetworkProxyController::isInterceptEnabled() const { return m_interceptEnabled; }
QString NetworkProxyController::getFilterText()      const { return m_filterText; }
QString NetworkProxyController::getFilterMethod()    const { return m_filterMethod; }
QString NetworkProxyController::getFilterStatusCode() const { return m_filterStatusCode; }
QString NetworkProxyController::getFilterContentType() const { return m_filterContentType; }
int     NetworkProxyController::getSelectedIndex()   const { return m_selectedIndex; }
int     NetworkProxyController::getDetailTab()       const { return m_detailTab; }
int     NetworkProxyController::getRequestTabIndex() const { return m_requestTabIndex; }
int     NetworkProxyController::getResponseTabIndex() const { return m_responseTabIndex; }
ProxyRequestModel* NetworkProxyController::getRequestModel() const { return m_requestModel; }
ProxyRulesManager* NetworkProxyController::getRulesManager() const { return m_rulesManager; }
ProxyCertManager*  NetworkProxyController::getCertManager()  const { return m_certManager; }
QString NetworkProxyController::getStatusMessage()   const { return m_statusMessage; }
int     NetworkProxyController::getRequestCount()    const { return m_requestModel ? m_requestModel->rowCount() : 0; }

// Helper: decode body text, handling base64 encoding flag
static QString decodeBodyText(const QJsonObject& data, const QString& bodyKey, const QString& base64FlagKey)
{
    QString body = data[bodyKey].toString();
    if (body.isEmpty())
        return body;

    bool isBase64 = data[base64FlagKey].toBool(false);
    if (isBase64) {
        QByteArray decoded = QByteArray::fromBase64(body.toLatin1());
        // Try UTF-8 decode first
        QString text = QString::fromUtf8(decoded);
        if (!text.contains(QChar(0xFFFD)))
            return text;
        // Show hex dump for first 512 bytes
        int showLen = qMin(decoded.size(), 512);
        QString hex;
        for (int i = 0; i < showLen; i += 16) {
            hex += QStringLiteral("%1  ").arg(i, 8, 16, QChar('0'));
            int lineEnd = qMin(i + 16, showLen);
            for (int j = i; j < lineEnd; ++j)
                hex += QStringLiteral("%1 ").arg(static_cast<unsigned char>(decoded[j]), 2, 16, QChar('0'));
            hex += QStringLiteral("  ");
            for (int j = i; j < lineEnd; ++j) {
                char c = decoded[j];
                hex += (c >= 32 && c < 127) ? QChar(c) : QChar('.');
            }
            hex += QChar('\n');
        }
        if (decoded.size() > showLen)
            hex += QStringLiteral("... (%1 more bytes)\n").arg(decoded.size() - showLen);
        return hex;
    }
    return body;
}

QString NetworkProxyController::getDetailText() const
{
    if (!m_requestModel || m_selectedIndex < 0 || m_selectedIndex >= m_requestModel->rowCount())
        return QString();

    QJsonObject req = m_requestModel->getRequestAt(m_selectedIndex);

    switch (m_detailTab) {
    case 0: { // Overview
        QString text;
        text += QStringLiteral("Method:       %1\n").arg(req["method"].toString());
        text += QStringLiteral("URL:          %1\n").arg(req["url"].toString());
        text += QStringLiteral("Protocol:     %1\n").arg(req["is_https"].toBool() ? "HTTPS" : "HTTP");
        text += QStringLiteral("Status:       %1\n").arg(req["status_code"].toInt());
        text += QStringLiteral("Duration:     %1 ms\n").arg(QString::number(req["duration"].toDouble() * 1000, 'f', 1));

        auto respLen = req["response_content_length"].toVariant().toLongLong();
        if (respLen > 0) {
            if (respLen < 1024)
                text += QStringLiteral("Size:         %1 B\n").arg(respLen);
            else if (respLen < 1048576)
                text += QStringLiteral("Size:         %1 KB\n").arg(QString::number(respLen / 1024.0, 'f', 1));
            else
                text += QStringLiteral("Size:         %1 MB\n").arg(QString::number(respLen / 1048576.0, 'f', 1));
        }

        text += QStringLiteral("Content-Type: %1\n").arg(req["response_content_type"].toString());
        text += QStringLiteral("Timestamp:    %1\n").arg(req["timestamp"].toString());
        text += QStringLiteral("Process:      %1\n").arg(req["process_name"].toString());
        text += QStringLiteral("Flow ID:      %1\n").arg(req["flow_id"].toString());
        return text;
    }
    case 1: { // Headers
        QString text;
        // Request headers
        QJsonObject reqHeaders = req["request_headers"].toObject();
        text += QStringLiteral("══════ Request Headers ══════\n\n");
        if (reqHeaders.isEmpty()) {
            text += QStringLiteral("(none)\n");
        } else {
            for (auto it = reqHeaders.begin(); it != reqHeaders.end(); ++it)
                text += QStringLiteral("%1: %2\n").arg(it.key(), it.value().toString());
        }

        // Response headers
        QJsonObject respHeaders = req["response_headers"].toObject();
        text += QStringLiteral("\n══════ Response Headers ══════\n\n");
        if (respHeaders.isEmpty()) {
            text += QStringLiteral("(waiting for response...)\n");
        } else {
            for (auto it = respHeaders.begin(); it != respHeaders.end(); ++it)
                text += QStringLiteral("%1: %2\n").arg(it.key(), it.value().toString());
        }
        return text;
    }
    case 2: { // Body
        QString text;
        // Request body
        auto reqContentLen = req["request_content_length"].toVariant().toLongLong();
        QString reqBody = decodeBodyText(req, QStringLiteral("request_body"), QStringLiteral("request_body_base64"));
        text += QStringLiteral("══════ Request Body (%1) ══════\n\n").arg(req["method"].toString());
        if (!reqBody.isEmpty()) {
            // Try to pretty-print JSON request body
            QJsonParseError reqErr;
            QJsonDocument reqDoc = QJsonDocument::fromJson(reqBody.toUtf8(), &reqErr);
            if (reqErr.error == QJsonParseError::NoError) {
                text += QString::fromUtf8(reqDoc.toJson(QJsonDocument::Indented));
            } else {
                text += reqBody;
            }
            if (!text.endsWith('\n'))
                text += QStringLiteral("\n");
        } else if (reqContentLen > 0) {
            text += QStringLiteral("(binary data, %1 bytes)\n").arg(reqContentLen);
        } else {
            QString method = req["method"].toString();
            if (method == QLatin1String("GET") || method == QLatin1String("HEAD")
                || method == QLatin1String("OPTIONS") || method == QLatin1String("DELETE")) {
                text += QStringLiteral("(%1 requests typically have no body)\n").arg(method);
            } else {
                text += QStringLiteral("(empty body)\n");
            }
        }

        // Response body
        int statusCode = req["status_code"].toInt();
        auto respLen = req["response_content_length"].toVariant().toLongLong();
        text += QStringLiteral("\n══════ Response Body (HTTP %1) ══════\n\n").arg(statusCode);
        QString respBody = decodeBodyText(req, QStringLiteral("response_body"), QStringLiteral("response_body_base64"));
        if (!respBody.isEmpty()) {
            // Try to pretty-print JSON
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(respBody.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError) {
                text += QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
            } else {
                text += respBody;
            }
        } else if (statusCode == 0) {
            text += QStringLiteral("(waiting for response...)\n");
        } else if (statusCode == 204 || statusCode == 304) {
            text += QStringLiteral("(HTTP %1 — no body expected)\n").arg(statusCode);
        } else if (respLen > 0) {
            text += QStringLiteral("(binary content, %1 bytes — not displayed)\n").arg(respLen);
        } else {
            text += QStringLiteral("(empty response body)\n");
        }
        return text;
    }
    default: {
        QJsonDocument doc(req);
        return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
    }
    }
}

QString NetworkProxyController::getRequestDetailText() const
{
    if (!m_requestModel || m_selectedIndex < 0 || m_selectedIndex >= m_requestModel->rowCount())
        return QString();

    QJsonObject req = m_requestModel->getRequestAt(m_selectedIndex);

    switch (m_requestTabIndex) {
    case 0: { // Headers
        QString text;
        QJsonObject reqHeaders = req["request_headers"].toObject();
        if (reqHeaders.isEmpty()) {
            text = QStringLiteral("(no headers)\n");
        } else {
            for (auto it = reqHeaders.begin(); it != reqHeaders.end(); ++it)
                text += QStringLiteral("%1: %2\n").arg(it.key(), it.value().toString());
        }
        return text;
    }
    case 1: { // Body
        auto reqContentLen = req["request_content_length"].toVariant().toLongLong();
        QString body = decodeBodyText(req, QStringLiteral("request_body"), QStringLiteral("request_body_base64"));

        if (!body.isEmpty()) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError)
                return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
            return body;
        }
        if (reqContentLen > 0)
            return QStringLiteral("(binary content, %1 bytes)\n").arg(reqContentLen);

        QString method = req["method"].toString();
        if (method == QLatin1String("GET") || method == QLatin1String("HEAD")
            || method == QLatin1String("OPTIONS") || method == QLatin1String("DELETE"))
            return QStringLiteral("(%1 requests typically have no body)\n").arg(method);
        return QStringLiteral("(empty body)\n");
    }
    case 2: { // Query / Params
        QString text;
        QUrl url(req["url"].toString());

        // URL parameters
        QUrlQuery query(url.query());
        auto items = query.queryItems(QUrl::FullyDecoded);
        if (items.isEmpty()) {
            text = QStringLiteral("(no query parameters)\n");
        } else {
            for (const auto& item : items)
                text += QStringLiteral("%1 = %2\n").arg(item.first, item.second);
        }

        // General info
        text += QStringLiteral("\n── General ──\n");
        text += QStringLiteral("Method:    %1\n").arg(req["method"].toString());
        text += QStringLiteral("URL:       %1\n").arg(req["url"].toString());
        text += QStringLiteral("Protocol:  %1\n").arg(req["is_https"].toBool() ? "HTTPS" : "HTTP");
        text += QStringLiteral("Timestamp: %1\n").arg(req["timestamp"].toString());
        text += QStringLiteral("Process:   %1\n").arg(req["process_name"].toString());
        return text;
    }
    default:
        return QString();
    }
}

QString NetworkProxyController::getResponseDetailText() const
{
    if (!m_requestModel || m_selectedIndex < 0 || m_selectedIndex >= m_requestModel->rowCount())
        return QString();

    QJsonObject req = m_requestModel->getRequestAt(m_selectedIndex);
    int statusCode = req["status_code"].toInt();

    if (statusCode == 0)
        return QStringLiteral("(waiting for response...)\n");

    switch (m_responseTabIndex) {
    case 0: { // Headers
        QString text;
        text += QStringLiteral("Status: HTTP %1\n\n").arg(statusCode);
        QJsonObject respHeaders = req["response_headers"].toObject();
        if (respHeaders.isEmpty()) {
            text += QStringLiteral("(no headers)\n");
        } else {
            for (auto it = respHeaders.begin(); it != respHeaders.end(); ++it)
                text += QStringLiteral("%1: %2\n").arg(it.key(), it.value().toString());
        }
        return text;
    }
    case 1: { // Body
        auto respLen = req["response_content_length"].toVariant().toLongLong();
        QString body = decodeBodyText(req, QStringLiteral("response_body"), QStringLiteral("response_body_base64"));

        if (!body.isEmpty()) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError)
                return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
            return body;
        }
        if (statusCode == 204 || statusCode == 304)
            return QStringLiteral("(HTTP %1 — no body expected)\n").arg(statusCode);
        if (respLen > 0)
            return QStringLiteral("(binary content, %1 bytes — not displayed)\n").arg(respLen);
        return QStringLiteral("(empty response body)\n");
    }
    case 2: { // Preview / Summary
        QString text;
        text += QStringLiteral("Status:       HTTP %1\n").arg(statusCode);
        text += QStringLiteral("Content-Type: %1\n").arg(req["response_content_type"].toString());

        auto respLen = req["response_content_length"].toVariant().toLongLong();
        if (respLen > 0) {
            if (respLen < 1024)
                text += QStringLiteral("Size:         %1 B\n").arg(respLen);
            else if (respLen < 1048576)
                text += QStringLiteral("Size:         %1 KB\n").arg(QString::number(respLen / 1024.0, 'f', 1));
            else
                text += QStringLiteral("Size:         %1 MB\n").arg(QString::number(respLen / 1048576.0, 'f', 1));
        }
        text += QStringLiteral("Duration:     %1 ms\n").arg(QString::number(req["duration"].toDouble() * 1000, 'f', 1));
        text += QStringLiteral("Flow ID:      %1\n").arg(req["flow_id"].toString());
        return text;
    }
    default:
        return QString();
    }
}

// ====================== Property setters ======================

void NetworkProxyController::setProxyPort(int port)
{
    if (m_proxyPort != port) {
        m_proxyPort = port;
        emit proxyPortChanged();
    }
}

void NetworkProxyController::setAutoSystemProxy(bool enabled)
{
    if (m_autoSystemProxy != enabled) {
        m_autoSystemProxy = enabled;
        emit autoSystemProxyChanged();
    }
}

void NetworkProxyController::setInterceptEnabled(bool enabled)
{
    if (m_interceptEnabled != enabled) {
        m_interceptEnabled = enabled;
        emit interceptEnabledChanged();

        if (m_viewModel)
            m_viewModel->setInterceptEnabled(enabled);
    }
}

void NetworkProxyController::setFilterText(const QString& text)
{
    if (m_filterText != text) {
        m_filterText = text;
        emit filterTextChanged();
    }
}

void NetworkProxyController::setFilterMethod(const QString& method)
{
    if (m_filterMethod != method) {
        m_filterMethod = method;
        emit filterMethodChanged();
    }
}

void NetworkProxyController::setFilterStatusCode(const QString& code)
{
    if (m_filterStatusCode != code) {
        m_filterStatusCode = code;
        emit filterStatusCodeChanged();
    }
}

void NetworkProxyController::setFilterContentType(const QString& type)
{
    if (m_filterContentType != type) {
        m_filterContentType = type;
        emit filterContentTypeChanged();
    }
}

void NetworkProxyController::setSelectedIndex(int index)
{
    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        emit selectedIndexChanged();
        emit detailTextChanged();
    }
}

void NetworkProxyController::setDetailTab(int tab)
{
    if (m_detailTab != tab) {
        m_detailTab = tab;
        emit detailTabChanged();
        emit detailTextChanged();
    }
}

void NetworkProxyController::setRequestTabIndex(int tab)
{
    if (m_requestTabIndex != tab) {
        m_requestTabIndex = tab;
        emit requestTabIndexChanged();
        emit detailTextChanged();
    }
}

void NetworkProxyController::setResponseTabIndex(int tab)
{
    if (m_responseTabIndex != tab) {
        m_responseTabIndex = tab;
        emit responseTabIndexChanged();
        emit detailTextChanged();
    }
}

// ====================== Start / Stop ======================

void NetworkProxyController::startProxy()
{
    if (m_proxyRunning || !m_viewModel)
        return;

    commonHead::viewModels::model::ProxyConfig config;
    config.proxyPort = m_proxyPort;
    config.autoSystemProxy = m_autoSystemProxy;

    m_viewModel->startProxy(config);
}

void NetworkProxyController::stopProxy()
{
    if (!m_viewModel)
        return;

    m_viewModel->stopProxy();
}

void NetworkProxyController::clearRequests()
{
    if (m_requestModel)
        m_requestModel->clear();

    m_selectedIndex = -1;
    emit selectedIndexChanged();
    emit detailTextChanged();
    emit requestCountChanged();
}

void NetworkProxyController::exportRequests(const QString& filePath)
{
    if (!m_requestModel)
        return;

    QJsonArray arr;
    for (int i = 0; i < m_requestModel->rowCount(); ++i) {
        arr.append(m_requestModel->getRequestAt(i));
    }

    // filePath may be a QUrl string from FileDialog (e.g. "file:///path/to/file.json")
    QString localPath = filePath;
    if (localPath.startsWith(QLatin1String("file://"))) {
        localPath = QUrl(localPath).toLocalFile();
    }

    QFile file(localPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
        file.close();
        setStatusMessage(tr("Exported %1 requests").arg(arr.size()));
    }
}

// ====================== Mock/Breakpoint/Blacklist/MapLocal/MapRemote/Throttle/Pattern ======================
// Delegated to ProxyRulesManager (m_rulesManager) — see ProxyRulesManager.cpp

// ====================== Intercept actions ======================

void NetworkProxyController::resumeRequest(const QString& flowId)
{
    if (m_viewModel)
        m_viewModel->resumeRequest(flowId.toStdString());

    QJsonObject patch;
    patch["flow_id"] = flowId;
    patch["is_intercepted"] = false;
    m_requestModel->addOrUpdateRequest(patch);
}

void NetworkProxyController::dropRequest(const QString& flowId)
{
    if (m_viewModel)
        m_viewModel->dropRequest(flowId.toStdString());

    QJsonObject patch;
    patch["flow_id"] = flowId;
    patch["is_intercepted"] = false;
    m_requestModel->addOrUpdateRequest(patch);
}

// ====================== Blacklist/MapLocal/MapRemote/Throttle ======================
// Delegated to ProxyRulesManager — see ProxyRulesManager.cpp

// ====================== Copy helpers ======================

void NetworkProxyController::copyRequestUrl()
{
    if (!m_requestModel || m_selectedIndex < 0)
        return;

    QJsonObject req = m_requestModel->getRequestAt(m_selectedIndex);
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard)
        clipboard->setText(req["url"].toString());
}

void NetworkProxyController::copyRequestCurl()
{
    if (!m_requestModel || m_selectedIndex < 0)
        return;

    QJsonObject req = m_requestModel->getRequestAt(m_selectedIndex);
    QString method = req["method"].toString();
    QString url = req["url"].toString();

    QString curl = QStringLiteral("curl -X %1 '%2'").arg(method, url);

    // Add headers
    QJsonObject headers = req["request_headers"].toObject();
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        curl += QStringLiteral(" -H '%1: %2'").arg(it.key(), it.value().toString());
    }

    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard)
        clipboard->setText(curl);
}

void NetworkProxyController::copyResponseBody()
{
    if (!m_requestModel || m_selectedIndex < 0)
        return;

    QJsonObject req = m_requestModel->getRequestAt(m_selectedIndex);
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard)
        clipboard->setText(req["response_body"].toString());
}

// ====================== ViewModel emitter slots ======================

void NetworkProxyController::onProxyStateChanged(int state)
{
    using PS = commonHead::viewModels::model::ProxyState;
    auto proxyState = static_cast<PS>(state);

    bool running = (proxyState == PS::Starting || proxyState == PS::Running);
    if (m_proxyRunning != running) {
        m_proxyRunning = running;
        emit proxyRunningChanged();
    }
}

void NetworkProxyController::onAddonConnectionChanged(bool connected)
{
    if (m_addonConnected != connected) {
        m_addonConnected = connected;
        emit addonConnectedChanged();
    }

    // Re-sync rules when addon reconnects
    if (connected && m_rulesManager)
        m_rulesManager->sendAllRules();
}

void NetworkProxyController::onRequestCaptured(const QString& /*flowId*/, const QString& rawJson)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(rawJson.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError)
        return;

    m_requestModel->addOrUpdateRequest(doc.object());
    emit requestCountChanged();
    if (m_selectedIndex >= 0)
        emit detailTextChanged();
}

void NetworkProxyController::onResponseCaptured(const QString& /*flowId*/, const QString& rawJson)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(rawJson.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError)
        return;

    m_requestModel->addOrUpdateRequest(doc.object());
    emit requestCountChanged();
    if (m_selectedIndex >= 0)
        emit detailTextChanged();
}

void NetworkProxyController::onRequestIntercepted(const QString& flowId, const QString& detailJson)
{
    QJsonObject patch;
    patch["flow_id"] = flowId;
    patch["is_intercepted"] = true;
    m_requestModel->addOrUpdateRequest(patch);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(detailJson.toUtf8(), &err);
    QJsonObject detail = (err.error == QJsonParseError::NoError) ? doc.object() : QJsonObject();

    emit interceptedRequest(flowId, detail);
}

void NetworkProxyController::onStatusMessage(const QString& message)
{
    setStatusMessage(message);
}

void NetworkProxyController::onCertStatusChanged(int /*status*/)
{
    // Re-emit so QML bindings re-evaluate
    if (m_certManager)
        emit m_certManager->caCertInstalledChanged();
}

void NetworkProxyController::onError(const QString& errorMessage)
{
    setStatusMessage(errorMessage);
}

// ====================== Internal helpers ======================

void NetworkProxyController::updateDetailText()
{
    emit detailTextChanged();
}

void NetworkProxyController::setStatusMessage(const QString& msg)
{
    if (m_statusMessage != msg) {
        m_statusMessage = msg;
        emit statusMessageChanged();
        UIVIEW_LOG_DEBUG("ProxyStatus: " << msg.toStdString());
    }
}
