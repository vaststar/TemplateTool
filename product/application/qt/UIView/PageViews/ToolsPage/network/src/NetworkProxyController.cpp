#include "PageViews/ToolsPage/network/include/NetworkProxyController.h"
#include "PageViews/ToolsPage/network/include/ProxyRequestModel.h"
#include "PageViews/ToolsPage/network/include/ProxyRulesManager.h"
#include "PageViews/ToolsPage/network/include/ProxyCertManager.h"
#include "LoggerDefine/LoggerDefine.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QClipboard>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>

NetworkProxyController::NetworkProxyController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create NetworkProxyController, this=" << (void*)this);
    m_requestModel = new ProxyRequestModel(this);
    m_rulesManager = new ProxyRulesManager(this);
    m_certManager  = new ProxyCertManager(this);

    // Wire cert manager status messages to our status
    connect(m_certManager, &ProxyCertManager::statusMessage,
            this, &NetworkProxyController::setStatusMessage);

    UIVIEW_LOG_DEBUG("[PROXY-DEBUG] requestModel created in ctor: " << (void*)m_requestModel);
}

NetworkProxyController::~NetworkProxyController()
{
    // Safe cleanup without emitting signals (object is being destroyed)
    if (m_autoSystemProxy)
        disableSystemProxy();

    if (m_addonSocket) {
        m_addonSocket->disconnect(this);
        m_addonSocket->disconnectFromHost();
        m_addonSocket = nullptr;
    }

    if (m_tcpServer) {
        m_tcpServer->disconnect(this);
        m_tcpServer->close();
    }

    if (m_process) {
        m_process->disconnect(this);
        m_process->terminate();
        m_process->waitForFinished(2000);
        if (m_process->state() != QProcess::NotRunning)
            m_process->kill();
    }
}

// ====================== init ======================

void NetworkProxyController::init()
{
    UIVIEW_LOG_DEBUG("NetworkProxyController::init");

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection,
            this, &NetworkProxyController::onNewTcpConnection);

    // Wire rules manager to use our sendCommand
    m_rulesManager->setSendCommandFn([this](const QJsonObject& cmd) {
        sendCommand(cmd);
    });
}

// ====================== Property getters ======================

bool    NetworkProxyController::isProxyRunning()     const { return m_proxyRunning; }
bool    NetworkProxyController::isAddonConnected()   const { return m_addonConnected; }
int     NetworkProxyController::getProxyPort()       const { return m_proxyPort; }
int     NetworkProxyController::getControlPort()     const { return m_controlPort; }
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

void NetworkProxyController::setControlPort(int port)
{
    if (m_controlPort != port) {
        m_controlPort = port;
        emit controlPortChanged();
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

        // Notify addon
        QJsonObject cmd;
        cmd["type"] = QStringLiteral("set_intercept");
        cmd["enabled"] = enabled;
        sendCommand(cmd);
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
    if (m_proxyRunning)
        return;

    // Re-connect TCP server signal (may have been disconnected by previous stopProxy)
    connect(m_tcpServer, &QTcpServer::newConnection,
            this, &NetworkProxyController::onNewTcpConnection, Qt::UniqueConnection);

    // Start TCP server for addon communication
    if (!m_tcpServer->listen(QHostAddress::LocalHost, static_cast<quint16>(m_controlPort))) {
        setStatusMessage(tr("Failed to start control server on port %1").arg(m_controlPort));
        UIVIEW_LOG_DEBUG("TCP server listen failed on port " << m_controlPort);
        return;
    }

    // Find addon executable
    QString addonPath = findAddonExecutable();
    if (addonPath.isEmpty()) {
        setStatusMessage(tr("Proxy addon executable not found"));
        m_tcpServer->close();
        return;
    }

    // Launch the addon process
    m_process = new QProcess(this);
    connect(m_process, &QProcess::started,
            this, &NetworkProxyController::onProcessStarted);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &NetworkProxyController::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &NetworkProxyController::onProcessError);
    connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
        if (m_process) {
            QByteArray errOutput = m_process->readAllStandardError();
            UIVIEW_LOG_DEBUG("[PROXY-STDERR] " << errOutput.toStdString());
        }
    });
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        if (m_process) {
            QByteArray stdOutput = m_process->readAllStandardOutput();
            UIVIEW_LOG_DEBUG("[PROXY-STDOUT] " << stdOutput.toStdString());
        }
    });

    QStringList args;

    // Development mode: if addon is a .py script, run via python
    QString program = addonPath;
    if (addonPath.endsWith(QStringLiteral(".py"))) {
#ifdef Q_OS_WIN
        program = QStringLiteral("python");
#else
        program = QStringLiteral("python3");
#endif
        args << addonPath;
    }

    args << QStringLiteral("--proxy-port") << QString::number(m_proxyPort)
         << QStringLiteral("--control-port") << QString::number(m_controlPort);

    UIVIEW_LOG_DEBUG("Launching: " << program.toStdString() << " " << args.join(" ").toStdString());
    m_process->start(program, args);

    m_proxyRunning = true;
    emit proxyRunningChanged();
    setStatusMessage(tr("Starting proxy on port %1...").arg(m_proxyPort));
}

void NetworkProxyController::stopProxy()
{
    if (!m_proxyRunning)
        return;

    // Disable system proxy first
    if (m_autoSystemProxy)
        disableSystemProxy();

    // Close addon socket — disconnect signals first to prevent callbacks during cleanup
    if (m_addonSocket) {
        m_addonSocket->disconnect(this);
        m_addonSocket->disconnectFromHost();
        m_addonSocket->deleteLater();
        m_addonSocket = nullptr;
    }

    // Close TCP server — disconnect signals to prevent onNewTcpConnection during waitForFinished
    if (m_tcpServer) {
        m_tcpServer->disconnect(this);
        m_tcpServer->close();
    }

    // Terminate process — disconnect signals first to prevent reentrant callback
    if (m_process) {
        m_process->disconnect(this);
        m_process->terminate();
        if (!m_process->waitForFinished(3000))
            m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }

    m_proxyRunning = false;
    m_addonConnected = false;
    emit proxyRunningChanged();
    emit addonConnectedChanged();
    setStatusMessage(tr("Proxy stopped"));
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
    QJsonObject cmd;
    cmd["type"] = QStringLiteral("resume_flow");
    cmd["flow_id"] = flowId;
    sendCommand(cmd);
    // Clear intercepted badge in model
    QJsonObject patch;
    patch["flow_id"] = flowId;
    patch["is_intercepted"] = false;
    m_requestModel->addOrUpdateRequest(patch);
}

void NetworkProxyController::dropRequest(const QString& flowId)
{
    QJsonObject cmd;
    cmd["type"] = QStringLiteral("drop_flow");
    cmd["flow_id"] = flowId;
    sendCommand(cmd);
    // Clear intercepted badge in model
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

// ====================== TCP callbacks ======================

void NetworkProxyController::onNewTcpConnection()
{
    UIVIEW_LOG_DEBUG("[PROXY-DEBUG] onNewTcpConnection called, hasPending=" << m_tcpServer->hasPendingConnections());
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket* socket = m_tcpServer->nextPendingConnection();
        if (m_addonSocket) {
            UIVIEW_LOG_DEBUG("[PROXY-DEBUG] Already have addon socket, rejecting new connection");
            // Only allow one addon connection
            socket->disconnectFromHost();
            socket->deleteLater();
            continue;
        }
        m_addonSocket = socket;
        m_tcpBuffer.clear();
        connect(m_addonSocket, &QTcpSocket::readyRead,
                this, &NetworkProxyController::onTcpDataReady);
        connect(m_addonSocket, &QTcpSocket::disconnected,
                this, &NetworkProxyController::onTcpDisconnected);

        m_addonConnected = true;
        emit addonConnectedChanged();
        UIVIEW_LOG_DEBUG("[PROXY-DEBUG] Addon socket connected from " << socket->peerAddress().toString().toStdString() << ":" << socket->peerPort());
        setStatusMessage(tr("Addon connected, proxy running on port %1").arg(m_proxyPort));

        // Enable system proxy after connection
        if (m_autoSystemProxy)
            enableSystemProxy();

        // Send initial rules via manager
        m_rulesManager->sendAllRules();
    }
}

void NetworkProxyController::onTcpDataReady()
{
    if (!m_addonSocket)
        return;

    QByteArray incoming = m_addonSocket->readAll();
    UIVIEW_LOG_DEBUG("[PROXY-DEBUG] onTcpDataReady: received " << incoming.size() << " bytes");
    m_tcpBuffer.append(incoming);

    // Parse newline-delimited JSON messages
    int msgCount = 0;
    while (true) {
        int idx = m_tcpBuffer.indexOf('\n');
        if (idx < 0)
            break;

        QByteArray line = m_tcpBuffer.left(idx);
        m_tcpBuffer.remove(0, idx + 1);

        if (line.isEmpty())
            continue;

        UIVIEW_LOG_DEBUG("[PROXY-DEBUG] Parsing JSON line (" << line.size() << " bytes): " << line.left(200).toStdString());

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError) {
            UIVIEW_LOG_DEBUG("[PROXY-DEBUG] JSON parse error: " << err.errorString().toStdString());
            continue;
        }

        handleAddonMessage(doc.object());
        ++msgCount;
    }
    UIVIEW_LOG_DEBUG("[PROXY-DEBUG] Processed " << msgCount << " messages, buffer remaining: " << m_tcpBuffer.size() << " bytes");
}

void NetworkProxyController::onTcpDisconnected()
{
    UIVIEW_LOG_DEBUG("[PROXY-DEBUG] onTcpDisconnected called");
    m_addonSocket = nullptr;
    m_addonConnected = false;
    emit addonConnectedChanged();
    setStatusMessage(tr("Addon disconnected"));
}

// ====================== Process callbacks ======================

void NetworkProxyController::onProcessStarted()
{
    UIVIEW_LOG_DEBUG("[PROXY-DEBUG] Proxy addon process started, PID=" << (m_process ? m_process->processId() : 0));
    setStatusMessage(tr("Proxy process started, waiting for addon connection..."));
}

void NetworkProxyController::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    UIVIEW_LOG_DEBUG("Proxy addon process finished: " << exitCode << " status: " << static_cast<int>(exitStatus));

    // Capture any remaining output before cleaning up
    if (m_process) {
        QByteArray remainingErr = m_process->readAllStandardError();
        QByteArray remainingOut = m_process->readAllStandardOutput();
        if (!remainingErr.isEmpty()) {
            UIVIEW_LOG_DEBUG("[PROXY-STDERR] " << remainingErr.toStdString());
        }
        if (!remainingOut.isEmpty()) {
            UIVIEW_LOG_DEBUG("[PROXY-STDOUT] " << remainingOut.toStdString());
        }
    }

    if (m_autoSystemProxy)
        disableSystemProxy();

    m_proxyRunning = false;
    m_addonConnected = false;
    emit proxyRunningChanged();
    emit addonConnectedChanged();
    setStatusMessage(tr("Proxy process exited (code %1)").arg(exitCode));

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_tcpServer->close();
}

void NetworkProxyController::onProcessError(QProcess::ProcessError error)
{
    UIVIEW_LOG_DEBUG("Proxy process error: " << static_cast<int>(error));
    setStatusMessage(tr("Proxy process error: %1").arg(static_cast<int>(error)));
}

// ====================== Internal helpers ======================

void NetworkProxyController::sendCommand(const QJsonObject& cmd)
{
    if (!m_addonSocket || !m_addonConnected)
        return;

    QJsonDocument doc(cmd);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');
    m_addonSocket->write(data);
    m_addonSocket->flush();
}

void NetworkProxyController::handleAddonMessage(const QJsonObject& msg)
{
    QString type = msg["type"].toString();
    UIVIEW_LOG_DEBUG("[PROXY-DEBUG] handleAddonMessage: type=" << type.toStdString()
                     << ", keys=" << QJsonDocument(msg).toJson(QJsonDocument::Compact).left(300).toStdString());

    if (type == QLatin1String("request") || type == QLatin1String("response")) {
        // Captured request/response data
        UIVIEW_LOG_DEBUG("[PROXY-DEBUG] -> Adding to model. Model ptr=" << (void*)m_requestModel
                         << ", current rowCount=" << (m_requestModel ? m_requestModel->rowCount() : -1));
        m_requestModel->addOrUpdateRequest(msg);
        UIVIEW_LOG_DEBUG("[PROXY-DEBUG] -> After add, rowCount=" << m_requestModel->rowCount());
        emit requestCountChanged();
        if (m_selectedIndex >= 0)
            emit detailTextChanged();

    } else if (type == QLatin1String("intercepted")) {
        // A request has been intercepted (breakpoint hit)
        QString flowId = msg["flow_id"].toString();
        // Mark the row in the model so capture list can show a badge
        QJsonObject patch;
        patch["flow_id"] = flowId;
        patch["is_intercepted"] = true;
        m_requestModel->addOrUpdateRequest(patch);
        emit interceptedRequest(flowId, msg);

    } else if (type == QLatin1String("error")) {
        setStatusMessage(tr("Addon error: %1").arg(msg["message"].toString()));

    } else if (type == QLatin1String("status")) {
        setStatusMessage(msg["message"].toString());
    }
}

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

// ====================== System proxy ======================

void NetworkProxyController::enableSystemProxy()
{
#if defined(Q_OS_MACOS)
    // Detect active network services (Wi-Fi, Ethernet, etc.)
    QProcess proc;
    proc.start(QStringLiteral("networksetup"), {"-listallnetworkservices"});
    proc.waitForFinished(3000);
    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    QStringList services;
    for (const QString& line : output.split('\n')) {
        QString trimmed = line.trimmed();
        // Skip header line and disabled services (prefixed with *)
        if (trimmed.isEmpty() || trimmed.startsWith('*') || trimmed.startsWith("An asterisk"))
            continue;
        services << trimmed;
    }
    if (services.isEmpty())
        services << QStringLiteral("Wi-Fi");

    m_proxyNetworkServices = services;
    UIVIEW_LOG_DEBUG("[PROXY-DEBUG] Setting system proxy on services: " << services.join(", ").toStdString());

    for (const QString& svc : services) {
        QProcess::execute(QStringLiteral("networksetup"),
            {"-setwebproxy", svc, "127.0.0.1", QString::number(m_proxyPort)});
        QProcess::execute(QStringLiteral("networksetup"),
            {"-setsecurewebproxy", svc, "127.0.0.1", QString::number(m_proxyPort)});
        QProcess::execute(QStringLiteral("networksetup"),
            {"-setwebproxystate", svc, "on"});
        QProcess::execute(QStringLiteral("networksetup"),
            {"-setsecurewebproxystate", svc, "on"});
    }
    UIVIEW_LOG_DEBUG("System proxy enabled (macOS) on " << services.size() << " services");
#elif defined(Q_OS_WIN)
    // Windows: set proxy via registry
    QString regPath = QStringLiteral("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
    QProcess::execute(QStringLiteral("reg"),
        {"add", regPath, "/v", "ProxyEnable", "/t", "REG_DWORD", "/d", "1", "/f"});
    QProcess::execute(QStringLiteral("reg"),
        {"add", regPath, "/v", "ProxyServer", "/t", "REG_SZ", "/d",
         QStringLiteral("127.0.0.1:%1").arg(m_proxyPort), "/f"});
    UIVIEW_LOG_DEBUG("System proxy enabled (Windows)");
#elif defined(Q_OS_LINUX)
    QString proxyUrl = QStringLiteral("http://127.0.0.1:%1").arg(m_proxyPort);
    QProcess::execute(QStringLiteral("gsettings"),
        {"set", "org.gnome.system.proxy", "mode", "manual"});
    QProcess::execute(QStringLiteral("gsettings"),
        {"set", "org.gnome.system.proxy.http", "host", "127.0.0.1"});
    QProcess::execute(QStringLiteral("gsettings"),
        {"set", "org.gnome.system.proxy.http", "port", QString::number(m_proxyPort)});
    QProcess::execute(QStringLiteral("gsettings"),
        {"set", "org.gnome.system.proxy.https", "host", "127.0.0.1"});
    QProcess::execute(QStringLiteral("gsettings"),
        {"set", "org.gnome.system.proxy.https", "port", QString::number(m_proxyPort)});
    UIVIEW_LOG_DEBUG("System proxy enabled (Linux)");
#endif
}

void NetworkProxyController::disableSystemProxy()
{
#if defined(Q_OS_MACOS)
    // Restore all services that were configured
    QStringList services = m_proxyNetworkServices;
    if (services.isEmpty())
        services << QStringLiteral("Wi-Fi");

    for (const QString& svc : services) {
        QProcess::execute(QStringLiteral("networksetup"),
            {"-setwebproxystate", svc, "off"});
        QProcess::execute(QStringLiteral("networksetup"),
            {"-setsecurewebproxystate", svc, "off"});
    }
    m_proxyNetworkServices.clear();
    UIVIEW_LOG_DEBUG("System proxy disabled (macOS)");
#elif defined(Q_OS_WIN)
    QString regPath = QStringLiteral("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
    QProcess::execute(QStringLiteral("reg"),
        {"add", regPath, "/v", "ProxyEnable", "/t", "REG_DWORD", "/d", "0", "/f"});
    UIVIEW_LOG_DEBUG("System proxy disabled (Windows)");
#elif defined(Q_OS_LINUX)
    QProcess::execute(QStringLiteral("gsettings"),
        {"set", "org.gnome.system.proxy", "mode", "none"});
    UIVIEW_LOG_DEBUG("System proxy disabled (Linux)");
#endif
}

QString NetworkProxyController::findAddonExecutable() const
{
    // Look for the addon in known locations relative to the app
    QStringList searchPaths;

    QString appDir = QCoreApplication::applicationDirPath();

#if defined(Q_OS_MACOS)
    // macOS: inside the .app bundle or alongside
    searchPaths << appDir + "/../Resources/proxy_addon/proxy_addon"
                << appDir + "/proxy_addon/proxy_addon"
                << appDir + "/../../../proxy_addon/proxy_addon";
#elif defined(Q_OS_WIN)
    searchPaths << appDir + "/proxy_addon/proxy_addon.exe"
                << appDir + "/../proxy_addon/proxy_addon.exe";
#else
    searchPaths << appDir + "/proxy_addon/proxy_addon"
                << appDir + "/../proxy_addon/proxy_addon";
#endif

    // Also check development path
#if defined(Q_OS_WIN)
    searchPaths << QDir::currentPath() + "/product/tools/proxy_addon/dist/proxy_addon/proxy_addon.exe";
#else
    searchPaths << QDir::currentPath() + "/product/tools/proxy_addon/dist/proxy_addon/proxy_addon";
#endif

    for (const QString& path : searchPaths) {
        UIVIEW_LOG_DEBUG("[PROXY-DEBUG] Checking addon path: " << path.toStdString() << " exists=" << QFile::exists(path));
        if (QFile::exists(path)) {
            UIVIEW_LOG_DEBUG("[PROXY-DEBUG] Found addon at: " << path.toStdString());
            return path;
        }
    }

    // Fallback: try Python script directly (development mode)
    QString pythonScript = QDir::currentPath() + "/product/tools/proxy_addon/proxy_addon.py";
    if (QFile::exists(pythonScript)) {
        UIVIEW_LOG_DEBUG("Using Python script directly: " << pythonScript.toStdString());
        return pythonScript;
    }

    UIVIEW_LOG_DEBUG("Proxy addon not found");
    return QString();
}

// ====================== Certificate helpers ======================
// Delegated to ProxyCertManager — see ProxyCertManager.cpp
