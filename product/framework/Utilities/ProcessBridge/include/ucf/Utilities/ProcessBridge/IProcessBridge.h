#pragma once

#include <cstdint>
#include <memory>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>
#include <ucf/Utilities/ProcessBridge/ProcessBridgeConfig.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

namespace ucf::utilities {

class IProcessBridgeCallback;

/// Manages a child process lifecycle: launch, monitor, and stop.
///
/// This class is purely about process management. It does NOT handle
/// communication (TCP, pipe, etc.) — that is the caller's responsibility.
///
/// Typical usage:
/// @code
///   auto bridge = IProcessBridge::create();
///   bridge->registerCallback(myCallback);
///
///   ProcessBridgeConfig config;
///   config.executablePath = "/path/to/child";
///   config.arguments = {"--port", "8080"};
///   bridge->start(config);
///
///   // ... process runs, callbacks fire on internal thread ...
///   // ... caller sets up TCP/IPC separately if needed ...
///
///   bridge->stop();  // graceful shutdown
/// @endcode
///
/// Thread safety:
///   - start(), stop(), and state queries are thread-safe.
///   - Callbacks are invoked on an internal monitor thread.
class Utilities_EXPORT IProcessBridge
    : public virtual INotificationHelper<IProcessBridgeCallback>
{
public:
    ~IProcessBridge() override = default;

    /// Launch the child process.
    /// @param config  Process configuration (executable, args, etc.)
    /// @return true if launch was initiated successfully
    virtual bool start(const ProcessBridgeConfig& config) = 0;

    /// Gracefully stop the child process.
    /// Sends terminate signal, waits up to stopTimeoutMs, then force-kills.
    /// No-op if process is not running.
    virtual void stop() = 0;

    /// Get the current process state.
    virtual ProcessState state() const = 0;

    /// Convenience: returns true if state is Starting or Running.
    virtual bool isRunning() const = 0;

    /// Get the child process PID. Returns 0 if not running.
    virtual int64_t processPid() const = 0;

    /// Create a ProcessBridge instance.
    static std::unique_ptr<IProcessBridge> create();
};

} // namespace ucf::utilities
