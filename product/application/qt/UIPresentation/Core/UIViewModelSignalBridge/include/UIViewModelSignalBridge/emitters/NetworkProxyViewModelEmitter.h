#pragma once

#include <atomic>

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <UIViewModelSignalBridge/UIViewModelSignalBridgeExport.h>
#include <commonhead/viewmodels/NetworkProxyViewModel/INetworkProxyViewModel.h>

namespace UIViewModelSignalBridge {

class UIViewModelSignalBridge_EXPORT NetworkProxyViewModelEmitter : public QObject,
                                                                public commonHead::viewModels::INetworkProxyViewModelCallback
{
    Q_OBJECT
public:
    explicit NetworkProxyViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    quint64 interceptGeneration() const
    {
        return m_interceptGeneration.load(std::memory_order_acquire);
    }

    // ── INetworkProxyViewModelCallback overrides ──

    void onProxyStateChanged(commonHead::viewModels::model::ProxyState state) override
    {
        using ProxyState = commonHead::viewModels::model::ProxyState;
        if (state != ProxyState::Running)
        {
            m_interceptGeneration.fetch_add(1, std::memory_order_acq_rel);
        }
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
        const quint64 generation = interceptGeneration();
        emit signals_onRequestIntercepted(QString::fromStdString(flowId),
                                          QString::fromStdString(detailJson), generation);
    }

    void onInterceptFinished(const std::string& flowId,
                             const std::string& reason) override
    {
        const quint64 generation = interceptGeneration();
        emit signals_onInterceptFinished(QString::fromStdString(flowId),
                                         QString::fromStdString(reason), generation);
    }

    void onProxyConfigResult(const std::string& sessionId,
                             const std::string& revision,
                             bool accepted,
                             const std::string& message) override
    {
        emit signals_onProxyConfigResult(QString::fromStdString(sessionId),
                                         QString::fromStdString(revision), accepted,
                                         QString::fromStdString(message));
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
    void signals_onRequestIntercepted(const QString& flowId, const QString& detailJson, quint64 generation);
    void signals_onInterceptFinished(const QString& flowId, const QString& reason, quint64 generation);
    void signals_onProxyConfigResult(const QString& sessionId, const QString& revision,
                                     bool accepted, const QString& message);
    void signals_onStatusMessage(const QString& message);
    void signals_onCertStatusChanged(int status);
    void signals_onError(const QString& errorMessage);

private:
    std::atomic<quint64> m_interceptGeneration{0};
};

} // namespace UIViewModelSignalBridge
