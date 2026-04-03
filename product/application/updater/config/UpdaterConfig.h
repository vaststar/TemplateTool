#pragma once

#include <filesystem>
#include <string>

namespace updater {

/// Configuration parsed from command-line arguments.
struct UpdaterConfig {
    std::string packagePath;          ///< Path to the ZIP package
    std::filesystem::path targetDir;  ///< Installation target directory
    int parentPid{0};                 ///< PID of the app to wait for
    bool restart{true};               ///< Whether to restart the app after install
};

/// Parse command-line arguments into UpdaterConfig.
/// Expected: --package <path> --target <path> --pid <num> [--restart]
UpdaterConfig parseArgs(int argc, char* argv[]);

} // namespace updater
