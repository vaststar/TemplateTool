#ifndef _WIN32

#include "PlatformUtils.h"
#include "../core/UpdaterLog.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <thread>

#include <csignal>
#include <sys/types.h>
#include <unistd.h>

namespace updater::platform {

bool waitForProcessExit(int pid, int timeoutSec)
{
    if (pid <= 0) {
        return true;
    }

    UPDATER_LOG("Waiting for process " << pid << " to exit...");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);

    while (std::chrono::steady_clock::now() < deadline) {
        // kill(pid, 0) checks existence without sending a signal
        if (kill(static_cast<pid_t>(pid), 0) != 0) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    UPDATER_WARN("Timed out waiting for process " << pid);
    return false;
}

int extractZipPackage(const std::string& zipPath, const std::string& destDir)
{
    std::error_code ec;
    std::filesystem::create_directories(destDir, ec);

    // unzip: -o = overwrite, -q = quiet
    std::ostringstream cmd;
    cmd << "unzip -o -q "
        << "'" << zipPath << "'"
        << " -d '" << destDir << "'";

    UPDATER_LOG("Running: " << cmd.str());
    int ret = std::system(cmd.str().c_str());

    if (ret != 0) {
        UPDATER_ERR("Extract command returned " << ret);
    }
    return ret;
}

bool launchApplication(const std::filesystem::path& targetDir)
{
#if defined(__APPLE__)
    // macOS: targetDir is the .app bundle
    std::string cmd = "open \"" + targetDir.string() + "\"";
    return std::system(cmd.c_str()) == 0;
#else
    // Linux
    auto exePath = targetDir / "bin" / "mainEntry";
    pid_t pid = fork();
    if (pid == 0) {
        execl(exePath.c_str(), exePath.c_str(), nullptr);
        _exit(1);
    }
    return pid > 0;
#endif
}

void fixPermissions(const std::filesystem::path& targetDir)
{
    std::error_code ec;

    for (auto& entry : std::filesystem::recursive_directory_iterator(targetDir)) {
        if (entry.is_regular_file()) {
            auto ext = entry.path().extension().string();
            if (ext.empty() || ext == ".sh" || ext == ".so" || ext == ".dylib") {
                std::filesystem::permissions(entry.path(),
                    std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec,
                    std::filesystem::perm_options::add, ec);
            }
        }
    }
}

} // namespace updater::platform

#endif // !_WIN32
