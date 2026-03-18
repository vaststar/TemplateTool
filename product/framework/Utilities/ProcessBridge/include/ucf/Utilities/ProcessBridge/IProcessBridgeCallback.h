#pragma once

#include <cstdint>
#include <string>

namespace ucf::utilities {

/// Callback interface for ProcessBridge lifecycle events.
///
/// All callbacks are invoked on the ProcessBridge's internal monitor thread.
/// Implementations must ensure thread safety when accessing shared state.
class IProcessBridgeCallback
{
public:
    virtual ~IProcessBridgeCallback() = default;

    /// Child process has been launched successfully.
    /// @param pid  Operating system process ID
    virtual void onProcessStarted(int64_t pid) = 0;

    /// Child process has exited.
    /// @param exitCode  Process exit code (0 typically means success)
    /// @param crashed   true if the process was force-killed or crashed
    virtual void onProcessStopped(int exitCode, bool crashed) = 0;

    /// Child process failed to start (e.g. executable not found, permission denied).
    /// @param errorMessage  Human-readable error description
    virtual void onProcessError(const std::string& errorMessage) = 0;

    /// Child process wrote to stdout.
    /// @param data  Raw output bytes (may not be a complete line)
    virtual void onStdout(const std::string& data) {}

    /// Child process wrote to stderr.
    /// @param data  Raw output bytes (may not be a complete line)
    virtual void onStderr(const std::string& data) {}
};

} // namespace ucf::utilities
