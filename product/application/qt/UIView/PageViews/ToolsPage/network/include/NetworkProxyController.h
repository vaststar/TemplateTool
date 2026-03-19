#pragma once

#include <QObject>
#include <QtQml>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include "UIViewBase/include/UIViewController.h"
#include "PageViews/ToolsPage/network/include/ProxyRequestModel.h"
#include "PageViews/ToolsPage/network/include/ProxyRulesManager.h"
#include "ViewModelSingalEmitter/NetworkProxyViewModelEmitter.h"

namespace commonHead::viewModels {
    class INetworkProxyViewModel;
}

/**
 * @brief Controller for the Network Proxy tool.
 *
 * Thin QML-facing bridge that delegates all proxy lifecycle, TCP,
 * system proxy, and certificate operations to INetworkProxyViewModel.
 * Receives ViewModel callbacks via NetworkProxyViewModelEmitter and
 * translates them to Q_PROPERTY / Qt signal updates for QML.
 *
 * Rule management is delegated to ProxyRulesManager.
 * Certificate status is queried from the ViewModel.
 */
class NetworkProxyController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    // --------------- Proxy state ---------------
    Q_PROPERTY(bool proxyRunning  READ isProxyRunning  NOTIFY proxyRunningChanged)
    Q_PROPERTY(bool addonConnected READ isAddonConnected NOTIFY addonConnectedChanged)
    Q_PROPERTY(int  proxyPort     READ getProxyPort     WRITE setProxyPort NOTIFY proxyPortChanged)
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

    // --------------- Delegated managers ---------------
    Q_PROPERTY(ProxyRulesManager* rulesManager READ getRulesManager CONSTANT)

    // --------------- Certificate (delegated to ViewModel) ---------------
    Q_PROPERTY(bool    caCertInstalled READ isCACertInstalled NOTIFY caCertInstalledChanged)
    Q_PROPERTY(bool    certInstalling  READ isCertInstalling  NOTIFY certInstallingChanged)
    Q_PROPERTY(QString caCertPath      READ getCACertPath     CONSTANT)

    // --------------- Status ---------------
    Q_PROPERTY(QString statusMessage READ getStatusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int     requestCount  READ getRequestCount  NOTIFY requestCountChanged)

public:
    explicit NetworkProxyController(QObject* parent = nullptr);
    ~NetworkProxyController() override;

    // Property getters
    bool    isProxyRunning()   const;
    bool    isAddonConnected() const;
    int     getProxyPort()     const;
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
    ProxyRulesManager* getRulesManager() const;
    bool    isCACertInstalled() const;
    bool    isCertInstalling()  const;
    QString getCACertPath()     const;
    QString getStatusMessage()  const;
    int     getRequestCount()  const;

    // Property setters
    void setProxyPort(int port);
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

    // Intercept actions (for breakpointed requests)
    Q_INVOKABLE void resumeRequest(const QString& flowId);
    Q_INVOKABLE void dropRequest(const QString& flowId);

    // Copy helpers
    Q_INVOKABLE void copyRequestUrl();
    Q_INVOKABLE void copyRequestCurl();
    Q_INVOKABLE void copyResponseBody();

    // Certificate actions (delegate to ViewModel)
    Q_INVOKABLE void installCACert();
    Q_INVOKABLE void checkCertStatus();
    Q_INVOKABLE void revealCACertInFolder();

protected:
    void init() override;

signals:
    void proxyRunningChanged();
    void addonConnectedChanged();
    void proxyPortChanged();
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
    void caCertInstalledChanged();
    void certInstallingChanged();
    void interceptedRequest(const QString& flowId, const QJsonObject& detail);

private slots:
    // ── ViewModel emitter slots ──
    void onProxyStateChanged(int state);
    void onAddonConnectionChanged(bool connected);
    void onRequestCaptured(const QString& flowId, const QString& rawJson);
    void onResponseCaptured(const QString& flowId, const QString& rawJson);
    void onRequestIntercepted(const QString& flowId, const QString& detailJson);
    void onStatusMessage(const QString& message);
    void onCertStatusChanged(int status);
    void onError(const QString& errorMessage);

private:
    void updateDetailText();
    void setStatusMessage(const QString& msg);

    // ViewModel
    std::shared_ptr<commonHead::viewModels::INetworkProxyViewModel> m_viewModel;
    std::shared_ptr<UIVMSignalEmitter::NetworkProxyViewModelEmitter> m_viewModelEmitter;

    // State (shadow of ViewModel state for QML property bindings)
    bool    m_proxyRunning     = false;
    bool    m_addonConnected   = false;
    int     m_proxyPort        = 8080;
    bool    m_autoSystemProxy  = true;
    bool    m_interceptEnabled = false;

    // Filter
    QString m_filterText;
    QString m_filterMethod;
    QString m_filterStatusCode;
    QString m_filterContentType;

    // Selection
    int m_selectedIndex    = -1;
    int m_detailTab        = 0;
    int m_requestTabIndex  = 0;
    int m_responseTabIndex = 0;

    // Model
    ProxyRequestModel* m_requestModel = nullptr;

    // Delegated managers
    ProxyRulesManager* m_rulesManager = nullptr;

    // Certificate state
    bool m_caCertInstalled = false;
    bool m_certInstalling  = false;

    // Status
    QString m_statusMessage;
};
