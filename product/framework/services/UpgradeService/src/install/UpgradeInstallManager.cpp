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

#ifdef __APPLE__
#include <mach-o/dyld.h>
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
    return std::filesystem::temp_directory_path() / upgrade::constants::kTempSubDir;
}

std::filesystem::path UpgradeInstallManager::getInstallDirectory() const
{
    auto execPath = getCurrentExecutablePath();
#if defined(__APPLE__)
    // macOS: the .app bundle's parent directory
    // execPath = /path/to/mainEntry.app/Contents/MacOS/mainEntry
    return execPath.parent_path().parent_path().parent_path(); // → mainEntry.app
#else
    // Windows & Linux: executable's parent directory (e.g. bin/)
    return execPath.parent_path();
#endif
}

std::filesystem::path UpgradeInstallManager::getCurrentExecutablePath() const
{
#if defined(__APPLE__) || defined(__linux__)
    char buf[4096];
#if defined(__APPLE__)
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        return std::filesystem::canonical(buf);
    }
#else
    auto len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        return std::filesystem::path(buf);
    }
#endif
    return {};
#elif defined(_WIN32)
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf);
#else
    return {};
#endif
}

bool UpgradeInstallManager::hasSufficientDiskSpace(int64_t requiredBytes) const
{
    try {
        auto targetDir = getInstallDirectory();
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
        auto targetDir = getInstallDirectory();
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
    auto execPath = getCurrentExecutablePath();
#if defined(__APPLE__)
    // macOS: updater is at .app/Contents/MacOS/updater (same dir as main exe)
    return execPath.parent_path() / upgrade::constants::kUpdaterBinaryName;
#else
    // Windows & Linux: updater alongside the main executable
    return execPath.parent_path() / upgrade::constants::kUpdaterBinaryName;
#endif
}

} // namespace ucf::service
