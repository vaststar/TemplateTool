#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

/// Current state of a managed child process.
enum class ProcessState {
    Idle,        ///< Not started yet
    Starting,    ///< Launch initiated, process may not be alive yet
    Running,     ///< Process confirmed alive
    Stopping,    ///< Graceful shutdown in progress
    Terminated   ///< Process ended (check callback for exit details)
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

    /// Whether to capture the child's stdout and fire onStdout callbacks.
    bool captureStdout = true;

    /// Whether to capture the child's stderr and fire onStderr callbacks.
    bool captureStderr = true;

    /// Whether to create a stdin pipe so the parent can write to the child's stdin.
    /// If false (default), the child inherits the parent's stdin.
    bool pipeStdin = false;

    /// Optional environment variables for the child process.
    /// Each pair is {key, value}. These are merged with (and override)
    /// the parent's environment. If empty, the child inherits as-is.
    std::vector<std::pair<std::string, std::string>> environment;
};

} // namespace ucf::utilities
