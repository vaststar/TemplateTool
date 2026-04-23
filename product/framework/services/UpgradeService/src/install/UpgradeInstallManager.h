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

    /// Extract callback: delivers the staging directory path on success
    using ExtractCallback = std::function<void(
        bool success,
        const std::string& stagingDir,
        model::UpgradeErrorCode errorCode,
        const std::string& errorMessage)>;

    /// Extract the ZIP package into a staging directory (<installDir>.staging/).
    /// The staging directory will contain the fully extracted new version.
    void extractPackageToStaging(const std::string& packagePath,
                                 ExtractCallback callback);

    /// Launch updater with a pre-extracted staging directory, then
    /// the app should exit.  Calls back with success/failure.
    void launchUpdaterAndExit(const std::string& stagingDir,
                              InstallCallback callback);

    /// Check for sufficient disk space (need ~2x package size for extraction)
    [[nodiscard]] bool hasSufficientDiskSpace(int64_t requiredBytes) const;

    /// On startup: check if a previous upgrade was interrupted and needs rollback
    void checkAndRecoverFromFailedUpgrade();

    /// Clean up any temp files from a previous upgrade
    void cleanupTempFiles();

    /// Reset state
    void reset();

private:
    /// Get the staging directory path (<installDir>.staging)
    [[nodiscard]] std::filesystem::path getStagingDirectory() const;

    /// Get the name of the updater entry inside the ZIP (platform-dependent)
    [[nodiscard]] std::string getUpdaterEntryName() const;

    /// Build command-line arguments for the updater process
    [[nodiscard]] std::vector<std::string> buildUpdaterArgs(
        const std::filesystem::path& stagingDir,
        const std::filesystem::path& targetDir) const;

    /// Get the temp directory used for upgrade operations
    [[nodiscard]] std::filesystem::path getTempDirectory() const;

    /// Get the path to the installed updater binary (platform-aware)
    [[nodiscard]] std::filesystem::path getUpdaterSourcePath() const;

private:
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
};

} // namespace ucf::service
