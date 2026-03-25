#pragma once

#include <string>

#include <ucf/Agents/AgentsCommonFile/AgentsExport.h>
#include <ucf/Utilities/SystemUtils/CertStoreUtils.h>

namespace ucf::agents {

/// Current state of the Network Proxy Agent.
enum class AgentState
{
    Idle,        ///< Not started
    Starting,    ///< TcpChannel listening + ProcessBridge launching
    Running,     ///< Addon process alive and connected via TCP
    Stopping,    ///< Shutting down
    Terminated   ///< Stopped (normal exit, crash, or startup failure)
};

/// Configuration for starting the Network Proxy Agent.
struct AgentConfig
{
    /// Port the mitmproxy addon listens on for HTTP(S) traffic.
    int proxyPort = 8080;

    /// Whether to configure the OS system proxy on start/stop.
    bool autoSystemProxy = true;

    /// Maximum time (ms) to wait for graceful exit before force-killing.
    int stopTimeoutMs = 3000;
};

/// Re-export for convenience — callers can use ucf::utilities::CertTrustStatus
/// and ucf::utilities::CertInstallResult without extra includes.
using CertTrustStatus = ucf::utilities::CertTrustStatus;
using CertInstallResult = ucf::utilities::CertInstallResult;

} // namespace ucf::agents
