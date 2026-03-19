#pragma once

#include <string>

#include <ucf/Utilities/NetworkProxyAgent/NetworkProxyAgentTypes.h>

namespace ucf::utilities {

/// Callback interface for NetworkProxyAgent events.
///
/// All callbacks are invoked on internal worker threads (ProcessBridge monitor
/// or TcpChannel I/O thread). Implementations must ensure thread safety.
class INetworkProxyAgentCallback
{
public:
    virtual ~INetworkProxyAgentCallback() = default;

    /// Agent lifecycle state changed.
    virtual void onAgentStateChanged(AgentState state) = 0;

    /// Addon process connected via TCP (agent is fully operational).
    virtual void onAddonConnected() = 0;

    /// Addon TCP connection lost (may precede process exit).
    virtual void onAddonDisconnected() = 0;

    /// A captured HTTP request was received from the addon.
    /// @param flowId   Unique flow identifier
    /// @param rawJson  Full JSON line from addon (type="request")
    virtual void onRequestCaptured(const std::string& flowId,
                                   const std::string& rawJson) = 0;

    /// A captured HTTP response was received from the addon.
    /// @param flowId   Unique flow identifier
    /// @param rawJson  Full JSON line from addon (type="response")
    virtual void onResponseCaptured(const std::string& flowId,
                                    const std::string& rawJson) = 0;

    /// A request was intercepted (breakpoint hit).
    /// @param flowId     Unique flow identifier
    /// @param detailJson Full JSON line from addon (type="intercepted")
    virtual void onRequestIntercepted(const std::string& flowId,
                                      const std::string& detailJson) = 0;

    /// Human-readable status message (informational).
    virtual void onStatusMessage(const std::string& message) = 0;

    /// An error occurred.
    virtual void onError(const std::string& errorMessage) = 0;
};

} // namespace ucf::utilities
