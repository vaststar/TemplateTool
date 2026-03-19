#pragma once

#include <QObject>
#include <QString>
#include <commonHead/viewModels/NetworkProxyViewModel/INetworkProxyViewModel.h>

namespace UIVMSignalEmitter {

class NetworkProxyViewModelEmitter : public QObject,
                                     public commonHead::viewModels::INetworkProxyViewModelCallback
{
    Q_OBJECT
public:
    explicit NetworkProxyViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    // ── INetworkProxyViewModelCallback overrides ──

    void onProxyStateChanged(commonHead::viewModels::model::ProxyState state) override
    {
        emit signals_onProxyStateChanged(static_cast<int>(state));
    }

    void onAddonConnectionChanged(bool connected) override
    {
        emit signals_onAddonConnectionChanged(connected);
    }

    void onRequestCaptured(const std::string& flowId,
                           const std::string& rawJson) override
    {
        emit signals_onRequestCaptured(QString::fromStdString(flowId),
                                       QString::fromStdString(rawJson));
    }

    void onResponseCaptured(const std::string& flowId,
                            const std::string& rawJson) override
    {
        emit signals_onResponseCaptured(QString::fromStdString(flowId),
                                        QString::fromStdString(rawJson));
    }

    void onRequestIntercepted(const std::string& flowId,
                              const std::string& detailJson) override
    {
        emit signals_onRequestIntercepted(QString::fromStdString(flowId),
                                          QString::fromStdString(detailJson));
    }

    void onStatusMessage(const std::string& message) override
    {
        emit signals_onStatusMessage(QString::fromStdString(message));
    }

    void onCertStatusChanged(commonHead::viewModels::model::CertStatus status) override
    {
        emit signals_onCertStatusChanged(static_cast<int>(status));
    }

    void onError(const std::string& errorMessage) override
    {
        emit signals_onError(QString::fromStdString(errorMessage));
    }

signals:
    void signals_onProxyStateChanged(int state);
    void signals_onAddonConnectionChanged(bool connected);
    void signals_onRequestCaptured(const QString& flowId, const QString& rawJson);
    void signals_onResponseCaptured(const QString& flowId, const QString& rawJson);
    void signals_onRequestIntercepted(const QString& flowId, const QString& detailJson);
    void signals_onStatusMessage(const QString& message);
    void signals_onCertStatusChanged(int status);
    void signals_onError(const QString& errorMessage);
};

} // namespace UIVMSignalEmitter
