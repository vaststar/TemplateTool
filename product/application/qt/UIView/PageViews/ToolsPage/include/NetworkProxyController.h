#pragma once

#include <QObject>
#include <QtQml>
#include <QProcess>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include "UIViewBase/include/UIViewController.h"
#include "PageViews/ToolsPage/include/ProxyRequestModel.h"

/**
 * @brief Controller for the Network Proxy tool.
 *
 * Manages the mitmproxy addon process, communicates via TCP socket,
 * collects captured HTTP(S) requests, and optionally configures
 * the system proxy on start/stop.
 */
class NetworkProxyController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    // --------------- Proxy state ---------------
    Q_PROPERTY(bool proxyRunning  READ isProxyRunning  NOTIFY proxyRunningChanged)
    Q_PROPERTY(bool addonConnected READ isAddonConnected NOTIFY addonConnectedChanged)
    Q_PROPERTY(int  proxyPort     READ getProxyPort     WRITE setProxyPort NOTIFY proxyPortChanged)
    Q_PROPERTY(int  controlPort   READ getControlPort   WRITE setControlPort NOTIFY controlPortChanged)
    Q_PROPERTY(bool autoSystemProxy READ getAutoSystemProxy WRITE setAutoSystemProxy NOTIFY autoSystemProxyChanged)

    // --------------- Intercept / breakpoint ---------------
    Q_PROPERTY(bool interceptEnabled READ isInterceptEnabled WRITE setInterceptEnabled NOTIFY interceptEnabledChanged)

    // --------------- Filter ---------------
    Q_PROPERTY(QString filterText READ getFilterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(QString filterMethod READ getFilterMethod WRITE setFilterMethod NOTIFY filterMethodChanged)
    Q_PROPERTY(QString filterStatusCode READ getFilterStatusCode WRITE setFilterStatusCode NOTIFY filterStatusCodeChanged)
    Q_PROPERTY(QString filterContentType READ getFilterContentType WRITE setFilterContentType NOTIFY filterContentTypeChanged)

    // --------------- Current request detail ---------------
    Q_PROPERTY(int     selectedIndex      READ getSelectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int     detailTab          READ getDetailTab     WRITE setDetailTab     NOTIFY detailTabChanged)
    Q_PROPERTY(QString detailText         READ getDetailText    NOTIFY detailTextChanged)
    Q_PROPERTY(int     requestTabIndex    READ getRequestTabIndex  WRITE setRequestTabIndex  NOTIFY requestTabIndexChanged)
    Q_PROPERTY(int     responseTabIndex   READ getResponseTabIndex WRITE setResponseTabIndex NOTIFY responseTabIndexChanged)
    Q_PROPERTY(QString requestDetailText  READ getRequestDetailText  NOTIFY detailTextChanged)
    Q_PROPERTY(QString responseDetailText READ getResponseDetailText NOTIFY detailTextChanged)

    // --------------- Model ---------------
    Q_PROPERTY(ProxyRequestModel* requestModel READ getRequestModel CONSTANT)

    // --------------- Status ---------------
    Q_PROPERTY(QString statusMessage READ getStatusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int     requestCount  READ getRequestCount  NOTIFY requestCountChanged)
    Q_PROPERTY(bool    caCertInstalled READ isCACertInstalled NOTIFY caCertInstalledChanged)
    Q_PROPERTY(bool    certInstalling READ isCertInstalling NOTIFY certInstallingChanged)

public:
    explicit NetworkProxyController(QObject* parent = nullptr);
    ~NetworkProxyController() override;

    // Property getters
    bool    isProxyRunning()   const;
    bool    isAddonConnected() const;
    int     getProxyPort()     const;
    int     getControlPort()   const;
    bool    getAutoSystemProxy() const;
    bool    isInterceptEnabled() const;
    QString getFilterText()    const;
    QString getFilterMethod()  const;
    QString getFilterStatusCode() const;
    QString getFilterContentType() const;
    int     getSelectedIndex() const;
    int     getDetailTab()     const;
    int     getRequestTabIndex() const;
    int     getResponseTabIndex() const;
    QString getDetailText()    const;
    QString getRequestDetailText() const;
    QString getResponseDetailText() const;
    ProxyRequestModel* getRequestModel() const;
    QString getStatusMessage() const;
    int     getRequestCount()  const;
    bool    isCertInstalling() const;

    // Property setters
    void setProxyPort(int port);
    void setControlPort(int port);
    void setAutoSystemProxy(bool enabled);
    void setInterceptEnabled(bool enabled);
    void setFilterText(const QString& text);
    void setFilterMethod(const QString& method);
    void setFilterStatusCode(const QString& code);
    void setFilterContentType(const QString& type);
    void setSelectedIndex(int index);
    void setDetailTab(int tab);
    void setRequestTabIndex(int tab);
    void setResponseTabIndex(int tab);

    // --------------- Q_INVOKABLE ---------------
    Q_INVOKABLE void startProxy();
    Q_INVOKABLE void stopProxy();
    Q_INVOKABLE void clearRequests();
    Q_INVOKABLE void exportRequests(const QString& filePath);

    // Mock rules
    Q_INVOKABLE void addMockRule(const QString& urlPattern, int statusCode,
                                  const QString& contentType, const QString& body,
                                  const QString& headers = QString());
    Q_INVOKABLE void removeMockRule(int index);
    Q_INVOKABLE void clearMockRules();
    Q_INVOKABLE QVariantList getMockRules() const;

    // Breakpoint rules
    Q_INVOKABLE void addBreakpointRule(const QString& urlPattern, const QString& method);
    Q_INVOKABLE void removeBreakpointRule(int index);
    Q_INVOKABLE void clearBreakpointRules();
    Q_INVOKABLE QVariantList getBreakpointRules() const;

    // Intercept actions (for breakpointed requests)
    Q_INVOKABLE void resumeRequest(const QString& flowId);
    Q_INVOKABLE void dropRequest(const QString& flowId);

    // Blacklist
    Q_INVOKABLE void addBlacklistRule(const QString& urlPattern);
    Q_INVOKABLE void removeBlacklistRule(int index);
    Q_INVOKABLE QVariantList getBlacklistRules() const;

    // Map local / remote
    Q_INVOKABLE void addMapLocalRule(const QString& urlPattern, const QString& localPath);
    Q_INVOKABLE void removeMapLocalRule(int index);
    Q_INVOKABLE QVariantList getMapLocalRules() const;

    Q_INVOKABLE void addMapRemoteRule(const QString& srcPattern, const QString& destUrl);
    Q_INVOKABLE void removeMapRemoteRule(int index);
    Q_INVOKABLE QVariantList getMapRemoteRules() const;

    // Throttle
    Q_INVOKABLE void setThrottle(bool enabled, int downloadKBps, int uploadKBps);

    // URL pattern testing
    Q_INVOKABLE QString testUrlPattern(const QString& pattern, const QString& testUrl);

    // Copy helpers
    Q_INVOKABLE void copyRequestUrl();
    Q_INVOKABLE void copyRequestCurl();
    Q_INVOKABLE void copyResponseBody();

    // Certificate helpers
    Q_INVOKABLE void installCACert();
    Q_INVOKABLE void revealCACertInFolder();
    Q_INVOKABLE bool isCACertInstalled() const;
    Q_INVOKABLE QString getCACertPath() const;

protected:
    void init() override;

signals:
    void proxyRunningChanged();
    void addonConnectedChanged();
    void proxyPortChanged();
    void controlPortChanged();
    void autoSystemProxyChanged();
    void interceptEnabledChanged();
    void filterTextChanged();
    void filterMethodChanged();
    void filterStatusCodeChanged();
    void filterContentTypeChanged();
    void selectedIndexChanged();
    void detailTabChanged();
    void requestTabIndexChanged();
    void responseTabIndexChanged();
    void detailTextChanged();
    void statusMessageChanged();
    void requestCountChanged();
    void interceptedRequest(const QString& flowId, const QJsonObject& detail);
    void caCertInstalledChanged();
    void certInstallingChanged();

private slots:
    void onNewTcpConnection();
    void onTcpDataReady();
    void onTcpDisconnected();
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    void sendCommand(const QJsonObject& cmd);
    void handleAddonMessage(const QJsonObject& msg);
    void updateDetailText();
    void setStatusMessage(const QString& msg);

    // System proxy helpers
    void enableSystemProxy();
    void disableSystemProxy();
    QString findAddonExecutable() const;

    // Proxy process
    QProcess*    m_process = nullptr;
    QTcpServer*  m_tcpServer = nullptr;
    QTcpSocket*  m_addonSocket = nullptr;
    QByteArray   m_tcpBuffer;

    // State
    bool    m_proxyRunning     = false;
    bool    m_addonConnected   = false;
    int     m_proxyPort        = 8080;
    int     m_controlPort      = 9876;
    bool    m_autoSystemProxy  = true;
    bool    m_interceptEnabled = false;

    // Filter
    QString m_filterText;
    QString m_filterMethod;
    QString m_filterStatusCode;
    QString m_filterContentType;

    // Selection
    int m_selectedIndex    = -1;
    int m_detailTab        = 0;  // 0=Overview, 1=Headers, 2=Body
    int m_requestTabIndex  = 0;  // 0=Headers, 1=Body, 2=Query
    int m_responseTabIndex = 0;  // 0=Headers, 1=Body, 2=Preview

    // Model
    ProxyRequestModel* m_requestModel = nullptr;

    // Rules (stored here, sent to addon via TCP)
    QJsonArray m_mockRules;
    QJsonArray m_breakpointRules;
    QJsonArray m_blacklistRules;
    QJsonArray m_mapLocalRules;
    QJsonArray m_mapRemoteRules;

    // Status
    QString m_statusMessage;

    // Certificate install
    bool       m_certInstalling = false;
    QProcess*  m_certProcess = nullptr;

    // Saved proxy state for restore
    QString m_savedProxyHost;
    QString m_savedProxyPort;
    bool    m_savedProxyEnabled = false;
    QStringList m_proxyNetworkServices;  // macOS: network services where proxy was set
};
