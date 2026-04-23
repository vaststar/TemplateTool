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

std::filesystem::path UpgradeInstallManager::extractUpdaterToTemp(
    const std::string& packagePath)
{
    auto tempDir = getTempDirectory();
    ucf::utilities::FilePathUtils::createDirectoriesUtf8(ucf::utilities::FilePathUtils::utf8FromPath(tempDir));

    ucf::utilities::ArchiveWrapper archiver;
    auto updaterName = getUpdaterEntryName();
    auto destPath = tempDir / updaterName;

    // CPack ZIPs nest files under a versioned prefix directory
    // (e.g. "Template-Factory-x.y.z-Windows/bin/updater.exe").
    // Find the actual entry path by suffix match.
    std::string fullEntryPath;
    auto entries = archiver.list(packagePath);
    for (const auto& entry : entries)
    {
        auto entryPath = std::filesystem::path(entry.name);
        if (entryPath.filename().string() == updaterName)
        {
            fullEntryPath = entry.name;
            break;
        }
    }

    if (fullEntryPath.empty())
    {
        throw std::runtime_error("Updater binary '" + updaterName + "' not found in package");
    }

    UPGRADE_LOG_INFO("Extracting updater from " << packagePath
                     << " entry=" << fullEntryPath << " → " << ucf::utilities::FilePathUtils::utf8FromPath(destPath));

    auto error = archiver.extractEntry(packagePath, fullEntryPath, ucf::utilities::FilePathUtils::utf8FromPath(destPath));
    if (error != ucf::utilities::ArchiveError::Success)
    {
        throw std::runtime_error(
            std::string("Failed to extract updater: ") +
            ucf::utilities::ArchiveWrapper::errorToString(error));
    }

    // Set executable permission on macOS/Linux
#ifndef _WIN32
    std::filesystem::permissions(destPath,
        std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec,
        std::filesystem::perm_options::add);
#endif

    return destPath;
}

std::vector<std::string> UpgradeInstallManager::buildUpdaterArgs(
    const std::string& packagePath,
    const std::filesystem::path& targetDir) const
{
    std::vector<std::string> args;
    args.emplace_back("--package");
    args.emplace_back(packagePath);
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
    const std::string& packagePath,
    InstallCallback callback)
{
    UPGRADE_LOG_INFO("Starting upgrade installation...");

    try {
        // 1. Extract updater to temp directory (not the install dir — avoids self-update problem)
        auto updaterPath = extractUpdaterToTemp(packagePath);

        // 2. Build arguments
        auto cf = mCoreFramework.lock();
        if (!cf) throw std::runtime_error("CoreFramework unavailable");
        auto clientInfo = cf->getService<IClientInfoService>().lock();
        if (!clientInfo) throw std::runtime_error("ClientInfoService unavailable");
        auto targetDir = ucf::utilities::FilePathUtils::pathFromUtf8(clientInfo->getInstallDirectory());
        auto args = buildUpdaterArgs(packagePath, targetDir);

        // 3. Write a marker file so we can detect interrupted upgrades on next start
        auto markerPath = getTempDirectory() / upgrade::constants::kUpgradeMarkerFileName;
        {
            std::ofstream marker(markerPath);
            marker << packagePath << "\n" << ucf::utilities::FilePathUtils::utf8FromPath(targetDir) << "\n";
        }

        // 4. Launch the updater process (detached — outlives this app)
        UPGRADE_LOG_INFO("Launching updater: " << ucf::utilities::FilePathUtils::utf8FromPath(updaterPath));
        UPGRADE_LOG_INFO("  target: " << ucf::utilities::FilePathUtils::utf8FromPath(targetDir));

        ucf::utilities::ProcessBridgeConfig config;
        config.executablePath = ucf::utilities::FilePathUtils::utf8FromPath(updaterPath);
        config.arguments = args;

        if (!ucf::utilities::IProcessBridge::launch(config)) {
            throw std::runtime_error("Failed to launch updater process");
        }

        UPGRADE_LOG_INFO("Updater launched successfully, app should exit now");
        callback(true, model::UpgradeErrorCode::None, "");
        // Caller (FSM/Manager) will trigger QApplication::quit()

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
        // Remove the marker; the old version is still running so we're okay
        std::filesystem::remove(markerPath);
        // Future: could trigger a notification to the user
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
