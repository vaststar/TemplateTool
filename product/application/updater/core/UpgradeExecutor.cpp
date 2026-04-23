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

    UPDATER_LOG("Starting upgrade");
    UPDATER_LOG("  package: " << config.packagePath);
    UPDATER_LOG("  target:  " << config.targetDir.string());

    // 1. Validate inputs
    if (!std::filesystem::exists(config.packagePath)) {
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

        // On Windows, file handles may linger briefly after the parent exits
        // (antivirus scanners, search indexer, DLL unload delays).
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

    // 4. Extract new version
    UPDATER_LOG("Extracting package...");

#if defined(__APPLE__)
    // macOS: targetDir is the .app bundle (e.g. /Applications/mainEntry.app).
    // Extract ZIP to a temp directory, then locate the .app inside and move it
    // to targetDir.  We can't extract directly to /Applications/ because the
    // ZIP contains a versioned top-level directory.
    auto tempExtractDir = std::filesystem::temp_directory_path()
                          / "template-factory-upgrade" / "extract";
    if (std::filesystem::exists(tempExtractDir)) {
        std::filesystem::remove_all(tempExtractDir, ec);
    }
    std::filesystem::create_directories(tempExtractDir, ec);

    int extractResult = platform::extractZipPackage(config.packagePath, tempExtractDir.string());
    if (extractResult != 0) {
        UPDATER_ERR("Extraction failed (exit code " << extractResult << ")");
        if (std::filesystem::exists(backupDir)) {
            UPDATER_LOG("Rolling back...");
            std::filesystem::rename(backupDir, config.targetDir, ec);
        }
        return 3;
    }

    // Find the .app bundle inside the extracted content.
    // Expected structure: <tempExtractDir>/<zipStem>/bin/mainEntry.app/
    std::filesystem::path appBundle;
    auto appName = config.targetDir.filename();  // e.g. "mainEntry.app"
    for (auto& entry : std::filesystem::recursive_directory_iterator(tempExtractDir)) {
        if (entry.is_directory() && entry.path().filename() == appName) {
            appBundle = entry.path();
            break;
        }
    }

    if (appBundle.empty()) {
        UPDATER_ERR("Could not find " << appName.string() << " in extracted package");
        if (std::filesystem::exists(backupDir)) {
            UPDATER_LOG("Rolling back...");
            std::filesystem::rename(backupDir, config.targetDir, ec);
        }
        std::filesystem::remove_all(tempExtractDir, ec);
        return 3;
    }

    UPDATER_LOG("Found app bundle: " << appBundle.string());
    std::filesystem::rename(appBundle, config.targetDir, ec);
    if (ec) {
        UPDATER_ERR("Failed to place app bundle: " << ec.message());
        if (std::filesystem::exists(backupDir)) {
            UPDATER_LOG("Rolling back...");
            std::filesystem::rename(backupDir, config.targetDir, ec);
        }
        std::filesystem::remove_all(tempExtractDir, ec);
        return 3;
    }
    std::filesystem::remove_all(tempExtractDir, ec);

#else
    // Windows & Linux: extract ZIP to the parent of targetDir.
    // The ZIP contains a versioned top-level directory that becomes targetDir.
    auto parentDir = config.targetDir.parent_path();
    int extractResult = platform::extractZipPackage(config.packagePath, parentDir.string());
    if (extractResult != 0) {
        UPDATER_ERR("Extraction failed (exit code " << extractResult << ")");
        if (std::filesystem::exists(backupDir)) {
            UPDATER_LOG("Rolling back...");
            std::filesystem::rename(backupDir, config.targetDir, ec);
        }
        return 3;
    }

    // 4b. The ZIP may contain a top-level directory with a different version
    //     number (e.g. "Template-Factory-2026.04.0.609-Windows") while
    //     targetDir is "Template-Factory-2026.03.0.611-Windows".
    //     Derive the extracted directory name from the ZIP filename stem.
    if (!std::filesystem::exists(config.targetDir)) {
        auto zipStem = std::filesystem::path(config.packagePath).stem();
        auto extracted = parentDir / zipStem;

        if (std::filesystem::exists(extracted) && extracted != config.targetDir) {
            UPDATER_LOG("Renaming extracted dir: " << extracted.string()
                        << " -> " << config.targetDir.string());
            std::filesystem::rename(extracted, config.targetDir, ec);
            if (ec) {
                UPDATER_ERR("Failed to rename extracted dir: " << ec.message());
                if (std::filesystem::exists(backupDir)) {
                    UPDATER_LOG("Rolling back...");
                    std::filesystem::rename(backupDir, config.targetDir, ec);
                }
                return 3;
            }
        }
    }
#endif

    // 5. Verify extraction
    if (!std::filesystem::exists(config.targetDir) || std::filesystem::is_empty(config.targetDir)) {
        UPDATER_ERR("Extraction produced empty target");
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
    if (std::filesystem::exists(config.packagePath)) {
        std::filesystem::remove(config.packagePath, ec);
    }
    auto markerPath = std::filesystem::temp_directory_path() / "template-factory-upgrade" / "upgrade_in_progress";
    if (std::filesystem::exists(markerPath)) {
        std::filesystem::remove(markerPath, ec);
    }

    UPDATER_LOG("Upgrade complete!");
    return 0;
}

} // namespace updater
