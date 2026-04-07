#pragma once

#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

/// Responsible for extracting the updater binary, launching it, and managing
/// rollback detection on next startup.
class UpgradeInstallManager final
{
public:
    explicit UpgradeInstallManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~UpgradeInstallManager();

    UpgradeInstallManager(const UpgradeInstallManager&) = delete;
    UpgradeInstallManager& operator=(const UpgradeInstallManager&) = delete;

    /// Install callback
    using InstallCallback = std::function<void(
        bool success,
        model::UpgradeErrorCode errorCode,
        const std::string& errorMessage)>;

    /// Extract updater from ZIP to a temp directory, launch it, then
    /// the app should exit.  Calls back with success/failure.
    void launchUpdaterAndExit(const std::string& packagePath,
                              InstallCallback callback);

    /// Get the current platform's install directory
    [[nodiscard]] std::filesystem::path getInstallDirectory() const;

    /// Get the current executable path
    [[nodiscard]] std::filesystem::path getCurrentExecutablePath() const;

    /// Check for sufficient disk space (need ~2x package size for extraction)
    [[nodiscard]] bool hasSufficientDiskSpace(int64_t requiredBytes) const;

    /// On startup: check if a previous upgrade was interrupted and needs rollback
    void checkAndRecoverFromFailedUpgrade();

    /// Clean up any temp files from a previous upgrade
    void cleanupTempFiles();

    /// Reset state
    void reset();

private:
    /// Extract the updater binary from the ZIP package to a temp directory
    [[nodiscard]] std::filesystem::path extractUpdaterToTemp(const std::string& packagePath);

    /// Get the name of the updater entry inside the ZIP (platform-dependent)
    [[nodiscard]] std::string getUpdaterEntryName() const;

    /// Build command-line arguments for the updater process
    [[nodiscard]] std::vector<std::string> buildUpdaterArgs(
        const std::string& packagePath,
        const std::filesystem::path& targetDir) const;

    /// Get the temp directory used for upgrade operations
    [[nodiscard]] std::filesystem::path getTempDirectory() const;

    /// Get the path to the installed updater binary (platform-aware)
    [[nodiscard]] std::filesystem::path getUpdaterSourcePath() const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
};

} // namespace ucf::service
