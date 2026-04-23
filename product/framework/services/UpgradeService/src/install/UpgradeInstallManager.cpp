#include "UpgradeInstallManager.h"
#include "../UpgradeConstants.h"
#include "../UpgradeServiceLogger.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/ArchiveUtils/ArchiveWrapper.h>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>
#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>

#include <csignal>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace ucf::service {

UpgradeInstallManager::UpgradeInstallManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
{
    UPGRADE_LOG_DEBUG("UpgradeInstallManager created");
}

UpgradeInstallManager::~UpgradeInstallManager()
{
    UPGRADE_LOG_DEBUG("UpgradeInstallManager destroyed");
}

std::filesystem::path UpgradeInstallManager::getTempDirectory() const
{
    if (auto cf = mCoreFramework.lock()) {
        if (auto clientInfo = cf->getService<IClientInfoService>().lock()) {
            return ucf::utilities::FilePathUtils::pathFromUtf8(clientInfo->getAppTempStoragePath())
                   / upgrade::constants::kTempSubDir;
        }
    }
    return std::filesystem::temp_directory_path() / upgrade::constants::kTempSubDir;
}

bool UpgradeInstallManager::hasSufficientDiskSpace(int64_t requiredBytes) const
{
    try {
        auto cf = mCoreFramework.lock();
        if (!cf) return false;
        auto clientInfo = cf->getService<IClientInfoService>().lock();
        if (!clientInfo) return false;
        auto targetDir = ucf::utilities::FilePathUtils::pathFromUtf8(clientInfo->getInstallDirectory());
        auto spaceInfo = std::filesystem::space(targetDir);
        return spaceInfo.available > static_cast<std::uintmax_t>(requiredBytes * 2);
    } catch (const std::exception& ex) {
        UPGRADE_LOG_ERROR("Failed to check disk space: " << ex.what());
        return false;
    }
}

std::string UpgradeInstallManager::getUpdaterEntryName() const
{
    return upgrade::constants::kUpdaterBinaryName;
}

std::filesystem::path UpgradeInstallManager::getStagingDirectory() const
{
    if (auto cf = mCoreFramework.lock()) {
        if (auto clientInfo = cf->getService<IClientInfoService>().lock()) {
            auto installDir = ucf::utilities::FilePathUtils::pathFromUtf8(clientInfo->getInstallDirectory());
            auto staging = installDir;
            staging += ".staging";
            return staging;
        }
    }
    return {};
}

void UpgradeInstallManager::extractPackageToStaging(
    const std::string& packagePath,
    ExtractCallback callback)
{
    UPGRADE_LOG_INFO("Extracting package to staging directory...");

    try {
        auto stagingDir = getStagingDirectory();
        if (stagingDir.empty()) {
            throw std::runtime_error("Could not determine staging directory");
        }

        // Clean up any previous staging directory
        std::error_code ec;
        if (std::filesystem::exists(stagingDir)) {
            std::filesystem::remove_all(stagingDir, ec);
            if (ec) {
                throw std::runtime_error("Failed to remove old staging dir: " + ec.message());
            }
        }
        std::filesystem::create_directories(stagingDir, ec);
        if (ec) {
            throw std::runtime_error("Failed to create staging dir: " + ec.message());
        }

        // Extract the full ZIP into the staging directory
        ucf::utilities::ArchiveWrapper archiver;
        auto error = archiver.extractAll(packagePath,
            ucf::utilities::FilePathUtils::utf8FromPath(stagingDir));
        if (error != ucf::utilities::ArchiveError::Success) {
            throw std::runtime_error(
                std::string("Failed to extract package: ") +
                ucf::utilities::ArchiveWrapper::errorToString(error));
        }

        // CPack ZIPs nest files under a versioned prefix directory
        // (e.g. "Template-Factory-x.y.z-Windows/").  If the staging dir
        // contains exactly one subdirectory, promote its contents up.
        std::filesystem::path singleChild;
        int childCount = 0;
        for (auto& entry : std::filesystem::directory_iterator(stagingDir)) {
            if (entry.is_directory()) {
                singleChild = entry.path();
            }
            ++childCount;
        }
        if (childCount == 1 && !singleChild.empty()) {
            UPGRADE_LOG_INFO("Promoting nested directory: "
                             << ucf::utilities::FilePathUtils::utf8FromPath(singleChild));
            // Move all children of the single subdirectory up to staging root
            for (auto& entry : std::filesystem::directory_iterator(singleChild)) {
                auto dest = stagingDir / entry.path().filename();
                std::filesystem::rename(entry.path(), dest, ec);
                if (ec) {
                    throw std::runtime_error(
                        "Failed to promote " + entry.path().filename().string() + ": " + ec.message());
                }
            }
            std::filesystem::remove(singleChild, ec);
        }

        UPGRADE_LOG_INFO("Package extracted to staging: "
                         << ucf::utilities::FilePathUtils::utf8FromPath(stagingDir));
        callback(true, ucf::utilities::FilePathUtils::utf8FromPath(stagingDir),
                 model::UpgradeErrorCode::None, "");

    } catch (const std::exception& ex) {
        UPGRADE_LOG_ERROR("Extract to staging failed: " << ex.what());
        // Clean up failed staging
        auto stagingDir = getStagingDirectory();
        if (!stagingDir.empty() && std::filesystem::exists(stagingDir)) {
            std::error_code ec;
            std::filesystem::remove_all(stagingDir, ec);
        }
        callback(false, "", model::UpgradeErrorCode::ExtractFailed, ex.what());
    }
}

std::vector<std::string> UpgradeInstallManager::buildUpdaterArgs(
    const std::filesystem::path& stagingDir,
    const std::filesystem::path& targetDir) const
{
    std::vector<std::string> args;
    args.emplace_back("--staging");
    args.emplace_back(ucf::utilities::FilePathUtils::utf8FromPath(stagingDir));
    args.emplace_back("--target");
    args.emplace_back(ucf::utilities::FilePathUtils::utf8FromPath(targetDir));
    args.emplace_back("--pid");
#ifdef _WIN32
    args.emplace_back(std::to_string(GetCurrentProcessId()));
#else
    args.emplace_back(std::to_string(getpid()));
#endif
    args.emplace_back("--restart");
    return args;
}

void UpgradeInstallManager::launchUpdaterAndExit(
    const std::string& stagingDir,
    InstallCallback callback)
{
    UPGRADE_LOG_INFO("Starting upgrade installation from staging...");

    try {
        auto stagingPath = ucf::utilities::FilePathUtils::pathFromUtf8(stagingDir);

        // 1. Locate updater binary inside staging dir (in bin/ subdirectory)
        auto updaterName = getUpdaterEntryName();
        auto updaterInStaging = stagingPath / "bin" / updaterName;
        if (!std::filesystem::exists(updaterInStaging)) {
            // Fallback: updater might be at staging root
            updaterInStaging = stagingPath / updaterName;
        }
        if (!std::filesystem::exists(updaterInStaging)) {
            throw std::runtime_error("Updater binary not found in staging directory");
        }

        // 2. Copy updater to temp dir (it can't run from staging since staging will be renamed)
        auto tempDir = getTempDirectory();
        ucf::utilities::FilePathUtils::createDirectoriesUtf8(
            ucf::utilities::FilePathUtils::utf8FromPath(tempDir));
        auto updaterPath = tempDir / updaterName;
        std::filesystem::copy_file(updaterInStaging, updaterPath,
            std::filesystem::copy_options::overwrite_existing);

#ifndef _WIN32
        std::filesystem::permissions(updaterPath,
            std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec,
            std::filesystem::perm_options::add);
#endif

        // 3. Build arguments
        auto cf = mCoreFramework.lock();
        if (!cf) throw std::runtime_error("CoreFramework unavailable");
        auto clientInfo = cf->getService<IClientInfoService>().lock();
        if (!clientInfo) throw std::runtime_error("ClientInfoService unavailable");
        auto targetDir = ucf::utilities::FilePathUtils::pathFromUtf8(clientInfo->getInstallDirectory());
        auto args = buildUpdaterArgs(stagingPath, targetDir);

        // 4. Write a marker file so we can detect interrupted upgrades on next start
        auto markerPath = tempDir / upgrade::constants::kUpgradeMarkerFileName;
        {
            std::ofstream marker(markerPath);
            marker << stagingDir << "\n" << ucf::utilities::FilePathUtils::utf8FromPath(targetDir) << "\n";
        }

        // 5. Launch the updater process (detached — outlives this app)
        UPGRADE_LOG_INFO("Launching updater: " << ucf::utilities::FilePathUtils::utf8FromPath(updaterPath));
        UPGRADE_LOG_INFO("  staging: " << stagingDir);
        UPGRADE_LOG_INFO("  target:  " << ucf::utilities::FilePathUtils::utf8FromPath(targetDir));

        ucf::utilities::ProcessBridgeConfig config;
        config.executablePath = ucf::utilities::FilePathUtils::utf8FromPath(updaterPath);
        config.arguments = args;

        if (!ucf::utilities::IProcessBridge::launch(config)) {
            throw std::runtime_error("Failed to launch updater process");
        }

        UPGRADE_LOG_INFO("Updater launched successfully, app should exit now");
        callback(true, model::UpgradeErrorCode::None, "");

    } catch (const std::exception& ex) {
        UPGRADE_LOG_ERROR("Failed to launch updater: " << ex.what());
        callback(false, model::UpgradeErrorCode::LaunchUpdaterFailed, ex.what());
    }
}

void UpgradeInstallManager::checkAndRecoverFromFailedUpgrade()
{
    auto markerPath = getTempDirectory() / upgrade::constants::kUpgradeMarkerFileName;
    if (std::filesystem::exists(markerPath)) {
        UPGRADE_LOG_WARN("Detected interrupted upgrade — previous upgrade may have failed");
        std::filesystem::remove(markerPath);
    }
    // Clean up any leftover staging directory from a failed upgrade
    auto stagingDir = getStagingDirectory();
    if (!stagingDir.empty() && std::filesystem::exists(stagingDir)) {
        UPGRADE_LOG_WARN("Cleaning up leftover staging directory");
        std::error_code ec;
        std::filesystem::remove_all(stagingDir, ec);
    }
}

void UpgradeInstallManager::cleanupTempFiles()
{
    auto tempDir = getTempDirectory();
    if (std::filesystem::exists(tempDir)) {
        std::error_code ec;
        std::filesystem::remove_all(tempDir, ec);
        if (ec) {
            UPGRADE_LOG_WARN("Failed to clean temp dir: " << ec.message());
        } else {
            UPGRADE_LOG_DEBUG("Cleaned temp dir: " << ucf::utilities::FilePathUtils::utf8FromPath(tempDir));
        }
    }
}

void UpgradeInstallManager::reset()
{
    cleanupTempFiles();
    // Also clean up any staging directory
    auto stagingDir = getStagingDirectory();
    if (!stagingDir.empty() && std::filesystem::exists(stagingDir)) {
        std::error_code ec;
        std::filesystem::remove_all(stagingDir, ec);
        if (ec) {
            UPGRADE_LOG_WARN("Failed to clean staging dir: " << ec.message());
        }
    }
    UPGRADE_LOG_DEBUG("UpgradeInstallManager reset");
}

std::filesystem::path UpgradeInstallManager::getUpdaterSourcePath() const
{
    std::filesystem::path execPath;
    if (auto cf = mCoreFramework.lock()) {
        if (auto clientInfo = cf->getService<IClientInfoService>().lock()) {
            execPath = ucf::utilities::FilePathUtils::pathFromUtf8(clientInfo->getExecutablePath());
        }
    }
    if (execPath.empty()) {
        return {};
    }
    // Updater is alongside the main executable on all platforms
    return execPath.parent_path() / upgrade::constants::kUpdaterBinaryName;
}

} // namespace ucf::service
