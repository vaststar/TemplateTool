#pragma once

#include <string>
#include <vector>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <ucf/Utilities/SystemUtils/CertStoreUtils.h>
#include <ucf/Utilities/NetworkProxyAgent/NetworkProxyAgentTypes.h>

namespace commonHead::viewModels::model {

/// Current state of the network proxy subsystem.
/// Maps directly from ucf::utilities::AgentState.
enum class ProxyState
{
    Idle,        ///< Not started
    Starting,    ///< Process launching + waiting for addon TCP connection
    Running,     ///< Process alive + addon connected
    Stopping,    ///< Shutting down
    Terminated   ///< Stopped (normal exit, crash, or startup failure)
};

/// Configuration for starting the proxy.
struct ProxyConfig
{
    /// Port the mitmproxy addon listens on for HTTP(S) traffic.
    int proxyPort = 8080;

    /// Whether to configure the OS system proxy on start/stop.
    bool autoSystemProxy = true;

    /// Maximum time (ms) to wait for graceful exit before force-killing.
    int stopTimeoutMs = 3000;
};

/// Certificate trust status.
/// Alias for the generic utility enum so existing ViewModel callbacks remain unchanged.
using CertStatus = ucf::utilities::CertTrustStatus;

/// Certificate install result.
using CertInstallResult = ucf::utilities::CertInstallResult;

/// Helper: convert AgentState → ProxyState
inline ProxyState toProxyState(ucf::utilities::AgentState agentState)
{
    switch (agentState)
    {
    case ucf::utilities::AgentState::Idle:       return ProxyState::Idle;
    case ucf::utilities::AgentState::Starting:   return ProxyState::Starting;
    case ucf::utilities::AgentState::Running:    return ProxyState::Running;
    case ucf::utilities::AgentState::Stopping:   return ProxyState::Stopping;
    case ucf::utilities::AgentState::Terminated: return ProxyState::Terminated;
    default:                                     return ProxyState::Idle;
    }
}

} // namespace commonHead::viewModels::model

