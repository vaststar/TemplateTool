#pragma once

#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::viewModels::model {

/// Current state of the network proxy subsystem.
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

/// Certificate trust status (ViewModel layer).
enum class CertStatus
{
    Unknown,       ///< Unable to determine
    FileNotFound,  ///< The certificate file does not exist
    NotTrusted,    ///< File exists but is NOT in the system trust store
    Trusted        ///< Certificate IS in the system trust store
};

/// Certificate install result (ViewModel layer).
enum class CertInstallResult
{
    Success,        ///< Certificate installed successfully
    FileNotFound,   ///< Certificate file does not exist
    ParseError,     ///< Certificate file could not be parsed (bad format)
    UserCancelled,  ///< User declined the OS security prompt
    Failed          ///< Other / unknown failure
};

} // namespace commonHead::viewModels::model

