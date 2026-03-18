#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

/// Current state of a managed child process.
enum class ProcessState {
    Idle,       ///< Not started yet
    Starting,   ///< Launch initiated, process may not be alive yet
    Running,    ///< Process confirmed alive
    Stopping,   ///< Graceful shutdown in progress
    Stopped,    ///< Process exited (check exit code)
    Error       ///< Failed to start or crashed unexpectedly
};

/// Configuration for launching a managed child process.
struct ProcessBridgeConfig {
    /// Path to the executable to launch.
    std::string executablePath;

    /// Command-line arguments passed to the child process.
    std::vector<std::string> arguments;

    /// Working directory for the child process.
    /// Empty string means inherit the parent's current working directory.
    std::string workingDirectory;

    /// Maximum time (ms) to wait for graceful exit before force-killing.
    int stopTimeoutMs = 3000;

    /// Whether to capture the child's stdout and deliver via callback.
    bool captureStdout = true;

    /// Whether to capture the child's stderr and deliver via callback.
    bool captureStderr = true;
};

} // namespace ucf::utilities
