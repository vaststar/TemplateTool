#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/NetworkProxyViewModel/INetworkProxyViewModel.h>

#include <ucf/Utilities/NetworkProxyAgent/INetworkProxyAgent.h>
#include <ucf/Utilities/NetworkProxyAgent/INetworkProxyAgentCallback.h>

namespace commonHead::viewModels {

/// Concrete implementation of INetworkProxyViewModel.
///
/// Thin wrapper around INetworkProxyAgent. All process management,
/// TCP communication, system proxy, and certificate logic is delegated
/// to the agent. This class translates agent callbacks into ViewModel
/// callbacks for the UI Controller layer.
class NetworkProxyViewModel final
    : public virtual INetworkProxyViewModel
    , public virtual commonHead::utilities::VMNotificationHelper<INetworkProxyViewModelCallback>
    , public ucf::utilities::INetworkProxyAgentCallback
    , public std::enable_shared_from_this<NetworkProxyViewModel>
{
public:
    explicit NetworkProxyViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    ~NetworkProxyViewModel() override;

    NetworkProxyViewModel(const NetworkProxyViewModel&) = delete;
    NetworkProxyViewModel& operator=(const NetworkProxyViewModel&) = delete;
    NetworkProxyViewModel(NetworkProxyViewModel&&) = delete;
    NetworkProxyViewModel& operator=(NetworkProxyViewModel&&) = delete;

    // ── IViewModel ──
    std::string getViewModelName() const override;

    // ── INetworkProxyViewModel ──
    void startProxy(const model::ProxyConfig& config) override;
    void stopProxy() override;
    model::ProxyState proxyState() const override;

    void sendCommand(const std::string& jsonCommand) override;
    void setInterceptEnabled(bool enabled) override;
    void resumeRequest(const std::string& flowId) override;
    void dropRequest(const std::string& flowId) override;

    void updateRules(const std::string& ruleType,
                     const std::string& rulesJson) override;

    model::CertStatus certStatus() const override;
    std::string caCertPath() const override;
    void installCACert() override;
    void checkCertStatus() override;

protected:
    void init() override;

private:
    // ── INetworkProxyAgentCallback ──
    void onAgentStateChanged(ucf::utilities::AgentState state) override;
    void onAddonConnected() override;
    void onAddonDisconnected() override;
    void onRequestCaptured(const std::string& flowId,
                           const std::string& rawJson) override;
    void onResponseCaptured(const std::string& flowId,
                            const std::string& rawJson) override;
    void onRequestIntercepted(const std::string& flowId,
                              const std::string& detailJson) override;
    void onStatusMessage(const std::string& message) override;
    void onError(const std::string& errorMessage) override;

    std::shared_ptr<ucf::utilities::INetworkProxyAgent> mAgent;
};

} // namespace commonHead::viewModels
