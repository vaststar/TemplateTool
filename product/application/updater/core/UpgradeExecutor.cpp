#include "UpgradeExecutor.h"
#include "UpdaterLog.h"
#include "../platform/PlatformUtils.h"

#include <chrono>
#include <filesystem>
#include <thread>

namespace updater {

int performUpgrade(const UpdaterConfig& config)
{
    std::error_code ec;

    UPDATER_LOG("Starting upgrade (staging-rename mode)");
    UPDATER_LOG("  staging: " << config.stagingDir.string());
    UPDATER_LOG("  target:  " << config.targetDir.string());

    // 1. Validate inputs
    if (!std::filesystem::exists(config.stagingDir) || std::filesystem::is_empty(config.stagingDir)) {
        UPDATER_ERR("Staging directory not found or empty: " << config.stagingDir.string());
        return 1;
    }
    if (config.targetDir.empty()) {
        UPDATER_ERR("Target directory not specified");
        return 1;
    }

    // 2. Wait for parent process to exit
    platform::waitForProcessExit(config.parentPid);

    // 3. Backup current installation
    std::filesystem::path backupDir = config.targetDir;
    backupDir += ".bak";

    if (std::filesystem::exists(backupDir)) {
        std::filesystem::remove_all(backupDir, ec);
        if (ec) {
            UPDATER_WARN("Failed to remove old backup: " << ec.message());
        }
    }

    if (std::filesystem::exists(config.targetDir)) {
        UPDATER_LOG("Backing up: " << config.targetDir.string() << " -> " << backupDir.string());

        // On Windows, file handles may linger briefly after the parent exits.
        // Retry the rename a few times with increasing delay.
        constexpr int kMaxRetries = 5;
        for (int attempt = 0; attempt < kMaxRetries; ++attempt) {
            ec.clear();
            std::filesystem::rename(config.targetDir, backupDir, ec);
            if (!ec) {
                break;
            }
            UPDATER_WARN("Backup attempt " << (attempt + 1) << " failed: " << ec.message());
            if (attempt + 1 < kMaxRetries) {
                std::this_thread::sleep_for(std::chrono::seconds(attempt + 1));
            }
        }
        if (ec) {
            UPDATER_ERR("Failed to backup after " << kMaxRetries << " attempts: " << ec.message());
            return 2;
        }
    }

    // 4. Rename staging directory to target (atomic on same partition)
    UPDATER_LOG("Renaming staging -> target");
    std::filesystem::rename(config.stagingDir, config.targetDir, ec);
    if (ec) {
        UPDATER_ERR("Failed to rename staging to target: " << ec.message());
        // Rollback
        if (std::filesystem::exists(backupDir)) {
            UPDATER_LOG("Rolling back...");
            std::filesystem::rename(backupDir, config.targetDir, ec);
        }
        return 3;
    }

    // 5. Verify
    if (!std::filesystem::exists(config.targetDir) || std::filesystem::is_empty(config.targetDir)) {
        UPDATER_ERR("Target directory empty after rename");
        if (std::filesystem::exists(backupDir)) {
            UPDATER_LOG("Rolling back...");
            std::filesystem::rename(backupDir, config.targetDir, ec);
        }
        return 4;
    }

    // 6. Fix permissions (no-op on Windows)
    platform::fixPermissions(config.targetDir);

    // 7. Launch the new version
    if (config.restart) {
        UPDATER_LOG("Launching new version...");
        if (!platform::launchApplication(config.targetDir)) {
            UPDATER_WARN("Failed to launch application");
        }
    }

    // 8. Cleanup
    UPDATER_LOG("Cleaning up...");
    if (std::filesystem::exists(backupDir)) {
        std::filesystem::remove_all(backupDir, ec);
    }
    auto markerPath = std::filesystem::temp_directory_path() / "template-factory-upgrade" / "upgrade_in_progress";
    if (std::filesystem::exists(markerPath)) {
        std::filesystem::remove(markerPath, ec);
    }

    UPDATER_LOG("Upgrade complete!");
    return 0;
}

} // namespace updater
