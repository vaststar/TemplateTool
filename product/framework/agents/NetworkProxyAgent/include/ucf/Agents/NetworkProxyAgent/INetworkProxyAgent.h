#pragma once

#include <memory>
#include <string>

#include <ucf/Agents/AgentsCommonFile/AgentsExport.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/Agents/NetworkProxyAgent/NetworkProxyAgentTypes.h>

namespace ucf::agents {

class INetworkProxyAgentCallback;

/// High-level orchestration of the mitmproxy addon process.
///
/// Combines ProcessBridge (child process), TcpChannel (control channel),
/// and SystemUtils (system proxy + certificate management) behind a
/// single start/stop interface.
///
/// Startup sequence:
///   1. TcpChannel listens on ephemeral port
///   2. ProcessBridge launches addon with --control-port <port>
///   3. Addon connects back → onAddonConnected → state = Running
///
/// Shutdown sequence:
///   1. Disable system proxy (if autoSystemProxy)
///   2. TcpChannel stop → terminates addon TCP connection
///   3. ProcessBridge stop → terminates addon process
///   4. State → Idle
///
/// Thread safety:
///   - start(), stop(), sendCommand(), and state queries are thread-safe.
///   - Callbacks are invoked on internal worker threads.
class Agents_EXPORT INetworkProxyAgent
    : public virtual ucf::utilities::INotificationHelper<INetworkProxyAgentCallback>
{
public:
    ~INetworkProxyAgent() override = default;

    // ── Lifecycle ──

    /// Start the proxy addon process and TCP control channel.
    /// @param config  Agent configuration
    /// @return true if startup was initiated (transitions to Starting)
    virtual bool start(const AgentConfig& config) = 0;

    /// Stop the proxy and clean up all resources.
    /// No-op if already Idle or Terminated.
    virtual void stop() = 0;

    /// Current agent state.
    [[nodiscard]] virtual AgentState state() const = 0;

    // ── Commands to addon ──

    /// Send a raw JSON command string to the addon (newline-delimited).
    virtual void sendCommand(const std::string& jsonCommand) = 0;

    /// Enable or disable request interception (breakpoints).
    virtual void setInterceptEnabled(bool enabled) = 0;

    /// Resume an intercepted request.
    virtual void resumeRequest(const std::string& flowId) = 0;

    /// Drop an intercepted request.
    virtual void dropRequest(const std::string& flowId) = 0;

    /// Update rules of a given type. rulesJson is a JSON array string.
    /// ruleType examples: "mock_rules", "breakpoint_rules", "blacklist",
    ///                    "map_local", "map_remote"
    virtual void updateRules(const std::string& ruleType,
                             const std::string& rulesJson) = 0;

    // ── Certificate management ──

    /// Current certificate trust status (cached).
    [[nodiscard]] virtual CertTrustStatus certTrustStatus() const = 0;

    /// Full path to the mitmproxy CA certificate file.
    [[nodiscard]] virtual std::string caCertPath() const = 0;

    /// Install the CA certificate into the system trust store.
    /// Blocks until OS dialog completes. Returns the install result.
    virtual CertInstallResult installCACert() = 0;

    /// Re-check whether the CA certificate is trusted.
    /// Result is delivered via onAgentStateChanged or can be queried via certTrustStatus().
    virtual void checkCertStatus() = 0;

    // ── Factory ──

    /// Create a NetworkProxyAgent instance.
    /// Returns shared_ptr because the implementation uses enable_shared_from_this
    /// internally (for registering as callback listener with ProcessBridge/TcpChannel).
    static std::shared_ptr<INetworkProxyAgent> create();
};

} // namespace ucf::agents
