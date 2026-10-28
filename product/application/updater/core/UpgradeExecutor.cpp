#include "UpgradeExecutor.h"
#include "UpgradeValidation.h"
#include "UpdaterLog.h"
#include "../platform/PlatformUtils.h"

#include <chrono>
#include <exception>
#include <filesystem>
#include <thread>

namespace updater {
namespace {

bool renameWithRetry(const std::filesystem::path& source,
                     const std::filesystem::path& destination)
{
    constexpr int kMaxRetries = 5;
    for (int attempt = 0; attempt < kMaxRetries; ++attempt) {
        std::error_code ec;
        std::filesystem::rename(source, destination, ec);
        if (!ec) {
            return true;
        }
        UPDATER_WARN("Rename attempt " << (attempt + 1) << " failed: "
                     << source.string() << " -> " << destination.string()
                     << ": " << ec.message());
        if (attempt + 1 < kMaxRetries) {
            std::this_thread::sleep_for(std::chrono::seconds(attempt + 1));
        }
    }
    return false;
}

} // namespace

UpgradeExitCode performUpgrade(const UpdaterConfig& config)
{
    try {
        if (config.parentPid <= 0) {
            UPDATER_ERR("A positive parent PID is required");
            return UpgradeExitCode::InvalidInput;
        }
        const auto paths = resolveUpgradePaths(config.stagingDir, config.targetDir);
        if (!paths) {
            return UpgradeExitCode::InvalidInput;
        }
        const auto& stagingDir = paths->staging;
        const auto& targetDir = paths->target;
        const auto& backupDir = paths->backup;

        UPDATER_LOG("Starting upgrade (staging-rename mode)");
        UPDATER_LOG("  staging: " << stagingDir.string());
        UPDATER_LOG("  target:  " << targetDir.string());

        // Finish staging preparation before moving the installed application.
        if (!validateApplicationLayout(stagingDir)) {
            return UpgradeExitCode::InvalidInput;
        }
        platform::fixPermissions(stagingDir);

        if (!platform::waitForProcessExit(config.parentPid)) {
            UPDATER_ERR("Cannot confirm parent exit; installation was not replaced");
            return UpgradeExitCode::ParentExitUnconfirmed;
        }

        // Keep a previous backup when the current installation is incomplete.
        if (!validateApplicationLayout(targetDir)) {
            UPDATER_ERR("Current installation is incomplete; preserving backup: " << backupDir.string());
            return UpgradeExitCode::BackupPreparationFailed;
        }

        std::error_code ec;
        std::filesystem::current_path(std::filesystem::temp_directory_path(), ec);
        if (ec) {
            UPDATER_ERR("Cannot change working directory: " << ec.message());
            return UpgradeExitCode::BackupPreparationFailed;
        }

        if (std::filesystem::exists(backupDir, ec)) {
            std::filesystem::remove_all(backupDir, ec);
            if (ec) {
                UPDATER_ERR("Cannot remove previous backup: " << ec.message());
                return UpgradeExitCode::BackupPreparationFailed;
            }
        } else if (ec) {
            UPDATER_ERR("Cannot inspect previous backup: " << ec.message());
            return UpgradeExitCode::BackupPreparationFailed;
        }

        UPDATER_LOG("Backing up current installation");
        if (!renameWithRetry(targetDir, backupDir)) {
            UPDATER_ERR("Failed to create installation backup");
            return UpgradeExitCode::BackupPreparationFailed;
        }

        UPDATER_LOG("Renaming staging -> target");
        if (!renameWithRetry(stagingDir, targetDir)) {
            UPDATER_ERR("Failed to replace installation; attempting rollback");
            std::error_code rollbackError;
            const bool targetExists = std::filesystem::exists(targetDir, rollbackError);
            if (rollbackError || targetExists) {
                UPDATER_ERR("Rollback stopped; target is occupied or cannot be inspected. Backup: "
                            << backupDir.string());
            } else if (renameWithRetry(backupDir, targetDir)) {
                UPDATER_LOG("Rollback completed");
            } else {
                UPDATER_ERR("Rollback failed; backup retained at " << backupDir.string());
            }
            return UpgradeExitCode::ReplacementFailed;
        }

        if (!validateApplicationLayout(targetDir)) {
            UPDATER_ERR("Installed application is incomplete; backup retained at " << backupDir.string());
            return UpgradeExitCode::InstalledLayoutInvalid;
        }

        if (config.restart) {
            UPDATER_LOG("Requesting application restart");
            if (!platform::launchApplication(targetDir)) {
                UPDATER_ERR("Application launch failed; backup retained at " << backupDir.string());
                return UpgradeExitCode::ApplicationLaunchFailed;
            }
        }

        // Retain one previous version even when the launch request succeeds.
        // A subsequent upgrade replaces it after all preparation checks pass.
        UPDATER_LOG("Previous version retained at " << backupDir.string());
        const auto markerPath = detail::getUpdaterDirectory() / "upgrade_in_progress";
        std::filesystem::remove(markerPath, ec);
        if (ec) {
            UPDATER_WARN("Failed to remove upgrade marker: " << ec.message());
        }
        UPDATER_LOG("Package replacement complete");
        return UpgradeExitCode::Success;
    } catch (const std::exception& ex) {
        UPDATER_ERR("Upgrade stopped: " << ex.what());
        return UpgradeExitCode::UnexpectedError;
    }
}

} // namespace updater
