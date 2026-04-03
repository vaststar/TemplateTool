#include "UpgradeExecutor.h"
#include "UpdaterLog.h"
#include "../platform/PlatformUtils.h"

#include <filesystem>

namespace updater {

int performUpgrade(const UpdaterConfig& config)
{
    namespace fs = std::filesystem;
    std::error_code ec;

    UPDATER_LOG("Starting upgrade");
    UPDATER_LOG("  package: " << config.packagePath);
    UPDATER_LOG("  target:  " << config.targetDir.string());

    // 1. Validate inputs
    if (!fs::exists(config.packagePath)) {
        UPDATER_ERR("Package not found: " << config.packagePath);
        return 1;
    }
    if (config.targetDir.empty()) {
        UPDATER_ERR("Target directory not specified");
        return 1;
    }

    // 2. Wait for parent process to exit
    platform::waitForProcessExit(config.parentPid);

    // 3. Backup current installation
    fs::path backupDir = config.targetDir;
    backupDir += ".bak";

    if (fs::exists(backupDir)) {
        fs::remove_all(backupDir, ec);
        if (ec) {
            UPDATER_WARN("Failed to remove old backup: " << ec.message());
        }
    }

    if (fs::exists(config.targetDir)) {
        UPDATER_LOG("Backing up: " << config.targetDir.string() << " -> " << backupDir.string());
        fs::rename(config.targetDir, backupDir, ec);
        if (ec) {
            UPDATER_ERR("Failed to backup: " << ec.message());
            return 2;
        }
    }

    // 4. Extract new version
    UPDATER_LOG("Extracting package...");
    auto parentDir = config.targetDir.parent_path();
    int extractResult = platform::extractZipPackage(config.packagePath, parentDir.string());
    if (extractResult != 0) {
        UPDATER_ERR("Extraction failed (exit code " << extractResult << ")");
        if (fs::exists(backupDir)) {
            UPDATER_LOG("Rolling back...");
            fs::rename(backupDir, config.targetDir, ec);
        }
        return 3;
    }

    // 5. Verify extraction
    if (!fs::exists(config.targetDir) || fs::is_empty(config.targetDir)) {
        UPDATER_ERR("Extraction produced empty target");
        if (fs::exists(backupDir)) {
            UPDATER_LOG("Rolling back...");
            fs::rename(backupDir, config.targetDir, ec);
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
    if (fs::exists(backupDir)) {
        fs::remove_all(backupDir, ec);
    }
    if (fs::exists(config.packagePath)) {
        fs::remove(config.packagePath, ec);
    }
    auto markerPath = fs::temp_directory_path() / "template-factory-upgrade" / "upgrade_in_progress";
    if (fs::exists(markerPath)) {
        fs::remove(markerPath, ec);
    }

    UPDATER_LOG("Upgrade complete!");
    return 0;
}

} // namespace updater
