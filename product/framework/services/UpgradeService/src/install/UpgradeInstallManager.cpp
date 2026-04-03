#include "UpgradeInstallManager.h"
#include "../UpgradeServiceLogger.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/ArchiveUtils/ArchiveWrapper.h>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>

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
    return std::filesystem::temp_directory_path() / "template-factory-upgrade";
}

std::filesystem::path UpgradeInstallManager::getInstallDirectory() const
{
#if defined(__APPLE__)
    // macOS: the .app bundle's parent directory
    auto execPath = getCurrentExecutablePath();
    // execPath = /path/to/mainEntry.app/Contents/MacOS/mainEntry
    return execPath.parent_path().parent_path().parent_path(); // → mainEntry.app
#elif defined(_WIN32)
    auto execPath = getCurrentExecutablePath();
    return execPath.parent_path(); // → bin/
#else
    return std::filesystem::path("/opt/template-factory");
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
#ifdef _WIN32
    return "updater.exe";
#else
    return "updater";
#endif
}

std::filesystem::path UpgradeInstallManager::extractUpdaterToTemp(
    const std::string& packagePath)
{
    auto tempDir = getTempDirectory();
    std::filesystem::create_directories(tempDir);

    ucf::utilities::ArchiveWrapper archiver;
    auto updaterEntry = getUpdaterEntryName();
    auto destPath = tempDir / updaterEntry;

    UPGRADE_LOG_INFO("Extracting updater from " << packagePath << " → " << destPath.string());

    auto error = archiver.extractEntry(packagePath, updaterEntry, destPath.string());
    if (error != ucf::utilities::ArchiveError::Success) {
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
    args.emplace_back(targetDir.string());
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
        auto markerPath = getTempDirectory() / "upgrade_in_progress";
        {
            std::ofstream marker(markerPath);
            marker << packagePath << "\n" << targetDir.string() << "\n";
        }

        // 4. Launch the updater process
        UPGRADE_LOG_INFO("Launching updater: " << updaterPath.string());
        UPGRADE_LOG_INFO("  target: " << targetDir.string());

        // Use ProcessLauncher-style fork+exec / CreateProcess
#ifdef _WIN32
        std::string cmdLine = "\"" + updaterPath.string() + "\"";
        for (const auto& arg : args) {
            cmdLine += " \"" + arg + "\"";
        }

        STARTUPINFOW si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};

        auto wCmdLine = std::wstring(cmdLine.begin(), cmdLine.end());
        if (!CreateProcessW(nullptr, wCmdLine.data(), nullptr, nullptr,
                            FALSE, 0, nullptr, nullptr, &si, &pi)) {
            throw std::runtime_error("CreateProcess failed: " + std::to_string(GetLastError()));
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
#else
        pid_t pid = fork();
        if (pid < 0) {
            throw std::runtime_error("fork() failed");
        }
        if (pid == 0) {
            // Child process: exec the updater
            std::vector<const char*> cArgs;
            cArgs.push_back(updaterPath.c_str());
            for (const auto& arg : args) {
                cArgs.push_back(arg.c_str());
            }
            cArgs.push_back(nullptr);
            execv(updaterPath.c_str(), const_cast<char* const*>(cArgs.data()));
            _exit(1); // exec failed
        }
#endif

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
    auto markerPath = getTempDirectory() / "upgrade_in_progress";
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
            UPGRADE_LOG_DEBUG("Cleaned temp dir: " << tempDir.string());
        }
    }
}

void UpgradeInstallManager::reset()
{
    cleanupTempFiles();
    UPGRADE_LOG_DEBUG("UpgradeInstallManager reset");
}

} // namespace ucf::service
