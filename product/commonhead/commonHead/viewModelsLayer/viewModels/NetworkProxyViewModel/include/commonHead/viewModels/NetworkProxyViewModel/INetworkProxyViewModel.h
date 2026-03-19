#pragma once

#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/NetworkProxyViewModel/NetworkProxyTypes.h>

namespace commonHead::viewModels {

// ════════════════════════════════════════════════════════════
//  Callback interface — UI Controller implements this
// ════════════════════════════════════════════════════════════

class COMMONHEAD_EXPORT INetworkProxyViewModelCallback
{
public:
    INetworkProxyViewModelCallback() = default;
    INetworkProxyViewModelCallback(const INetworkProxyViewModelCallback&) = delete;
    INetworkProxyViewModelCallback(INetworkProxyViewModelCallback&&) = delete;
    INetworkProxyViewModelCallback& operator=(const INetworkProxyViewModelCallback&) = delete;
    INetworkProxyViewModelCallback& operator=(INetworkProxyViewModelCallback&&) = delete;
    virtual ~INetworkProxyViewModelCallback() = default;

public:
    /// Proxy lifecycle state changed.
    virtual void onProxyStateChanged(model::ProxyState state) = 0;

    /// Addon TCP connection established or lost.
    virtual void onAddonConnectionChanged(bool connected) = 0;

    /// A captured HTTP request was received from the addon.
    virtual void onRequestCaptured(const std::string& flowId,
                                   const std::string& rawJson) = 0;

    /// A captured HTTP response was received from the addon.
    virtual void onResponseCaptured(const std::string& flowId,
                                    const std::string& rawJson) = 0;

    /// A request was intercepted (breakpoint hit).
    virtual void onRequestIntercepted(const std::string& flowId,
                                      const std::string& detailJson) = 0;

    /// Human-readable status message for the UI.
    virtual void onStatusMessage(const std::string& message) = 0;

    /// Certificate trust status changed.
    virtual void onCertStatusChanged(model::CertStatus status) = 0;

    /// An error occurred.
    virtual void onError(const std::string& errorMessage) = 0;
};

// ════════════════════════════════════════════════════════════
//  ViewModel interface
// ════════════════════════════════════════════════════════════

class COMMONHEAD_EXPORT INetworkProxyViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<INetworkProxyViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    INetworkProxyViewModel(const INetworkProxyViewModel&) = delete;
    INetworkProxyViewModel(INetworkProxyViewModel&&) = delete;
    INetworkProxyViewModel& operator=(const INetworkProxyViewModel&) = delete;
    INetworkProxyViewModel& operator=(INetworkProxyViewModel&&) = delete;
    virtual ~INetworkProxyViewModel() = default;

public:
    virtual std::string getViewModelName() const = 0;

    // ── Lifecycle ──

    /// Start the proxy addon process and TCP control channel.
    virtual void startProxy(const model::ProxyConfig& config) = 0;

    /// Stop the proxy addon process and clean up.
    virtual void stopProxy() = 0;

    /// Current proxy state.
    [[nodiscard]] virtual model::ProxyState proxyState() const = 0;

    // ── Commands to addon ──

    /// Send a raw JSON command string to the addon (newline-delimited).
    virtual void sendCommand(const std::string& jsonCommand) = 0;

    /// Enable or disable request interception.
    virtual void setInterceptEnabled(bool enabled) = 0;

    /// Resume an intercepted request.
    virtual void resumeRequest(const std::string& flowId) = 0;

    /// Drop an intercepted request.
    virtual void dropRequest(const std::string& flowId) = 0;

    // ── Rules ──

    /// Update rules of a given type. rulesJson is a JSON array string.
    virtual void updateRules(const std::string& ruleType,
                             const std::string& rulesJson) = 0;

    // ── Certificate management ──

    /// Current certificate trust status (cached).
    [[nodiscard]] virtual model::CertStatus certStatus() const = 0;

    /// Full path to the mitmproxy CA certificate file.
    [[nodiscard]] virtual std::string caCertPath() const = 0;

    /// Install the CA certificate into the system trust store.
    virtual void installCACert() = 0;

    /// Re-check whether the CA certificate is installed.
    virtual void checkCertStatus() = 0;

    // ── Factory ──

    static std::shared_ptr<INetworkProxyViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
