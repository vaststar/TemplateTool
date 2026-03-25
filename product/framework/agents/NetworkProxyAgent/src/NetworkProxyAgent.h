#pragma once

#include <atomic>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <string>

#include <ucf/Agents/NetworkProxyAgent/INetworkProxyAgent.h>
#include <ucf/Agents/NetworkProxyAgent/INetworkProxyAgentCallback.h>
#include <ucf/Agents/NetworkProxyAgent/NetworkProxyAgentTypes.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

#include <ucf/Utilities/ProcessBridge/IProcessBridge.h>
#include <ucf/Utilities/ProcessBridge/IProcessBridgeCallback.h>
#include <ucf/Utilities/TcpChannel/ITcpChannel.h>
#include <ucf/Utilities/TcpChannel/ITcpChannelCallback.h>

namespace ucf::agents {

/// Concrete implementation of INetworkProxyAgent.
///
/// Orchestrates ProcessBridge (addon process), TcpChannel (control channel),
/// SystemUtils (system proxy + cert store), AddonLocator, and AddonProtocol.
///
/// Implements IProcessBridgeCallback and ITcpChannelCallback to receive events
/// from the lower-level utilities and translate them into INetworkProxyAgentCallback
/// notifications.
class NetworkProxyAgent final
    : public virtual INetworkProxyAgent
    , public virtual ucf::utilities::NotificationHelper<INetworkProxyAgentCallback>
    , public ucf::utilities::IProcessBridgeCallback
    , public ucf::utilities::ITcpChannelCallback
    , public std::enable_shared_from_this<NetworkProxyAgent>
{
public:
    NetworkProxyAgent();
    ~NetworkProxyAgent() override;

    NetworkProxyAgent(const NetworkProxyAgent&) = delete;
    NetworkProxyAgent& operator=(const NetworkProxyAgent&) = delete;
    NetworkProxyAgent(NetworkProxyAgent&&) = delete;
    NetworkProxyAgent& operator=(NetworkProxyAgent&&) = delete;

    // ── INetworkProxyAgent ──
    bool start(const AgentConfig& config) override;
    void stop() override;
    AgentState state() const override;

    void sendCommand(const std::string& jsonCommand) override;
    void setInterceptEnabled(bool enabled) override;
    void resumeRequest(const std::string& flowId) override;
    void dropRequest(const std::string& flowId) override;
    void updateRules(const std::string& ruleType,
                     const std::string& rulesJson) override;

    CertTrustStatus certTrustStatus() const override;
    std::string caCertPath() const override;
    CertInstallResult installCACert() override;
    void checkCertStatus() override;

private:
    enum class ShutdownReason
    {
        UserRequest,
        TcpDisconnected,
        ProcessExited
    };

    // ── IProcessBridgeCallback ──
    void onProcessStarted(int64_t pid) override;
    void onProcessStopped(int exitCode, bool crashed) override;
    void onProcessError(const std::string& errorMessage) override;
    void onStdout(const std::string& data) override;
    void onStderr(const std::string& data) override;

    // ── ITcpChannelCallback ──
    void onClientConnected() override;
    void onClientDisconnected() override;
    void onDataReceived(const std::string& data) override;
    void onError(const std::string& errorMessage) override;

    // ── Internal ──
    void handleAddonMessage(const std::string& jsonLine);
    void doCleanup();
    void failStart(const std::string& errorMessage);
    bool beginShutdown(ShutdownReason reason);
    void finalizeShutdown(ShutdownReason reason,
                          int exitCode = 0,
                          bool crashed = false);

    /// Attempt to transition mState to @p to.
    /// Valid source states are defined internally per target state.
    bool tryTransition(AgentState to);

    /// CAS helper: try transitioning from any of @p fromStates to @p to.
    bool casFrom(std::initializer_list<AgentState> fromStates, AgentState to);

    std::unique_ptr<ucf::utilities::IProcessBridge> mProcessBridge;
    std::unique_ptr<ucf::utilities::ITcpChannel> mTcpChannel;

    std::atomic<AgentState> mState{AgentState::Idle};
    std::atomic<CertTrustStatus> mCertStatus{CertTrustStatus::Unknown};
    AgentConfig mConfig;

    std::mutex mLifecycleMutex;  ///< Protects mConfig writes in start()
    std::mutex mBufferMutex;
    std::string mTcpBuffer;     ///< Accumulates partial newline-delimited JSON
};

} // namespace ucf::agents
