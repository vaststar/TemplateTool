#ifndef _WIN32

#include "PlatformUtils.h"
#include "../core/UpdaterLog.h"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <thread>

#include <csignal>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

namespace updater::platform {

bool waitForProcessExit(int pid, int timeoutSec)
{
    if (pid <= 0) {
        UPDATER_ERR("Invalid parent PID: " << pid);
        return false;
    }

    UPDATER_LOG("Waiting for process " << pid << " to exit...");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);

    while (std::chrono::steady_clock::now() < deadline) {
        // kill(pid, 0) checks existence without sending a signal
        if (kill(static_cast<pid_t>(pid), 0) != 0) {
            const int error = errno;
            if (error == ESRCH) {
                return true;
            }
            if (error != EINTR) {
                UPDATER_ERR("Cannot inspect parent process: " << std::strerror(error));
                return false;
            }
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

std::filesystem::path getApplicationExecutablePath(const std::filesystem::path& root)
{
#if defined(__APPLE__)
    return root / "Contents" / "MacOS" / "mainEntry";
#else
    return root / "bin" / "mainEntry";
#endif
}

bool launchApplication(const std::filesystem::path& targetDir)
{
#if defined(__APPLE__)
    std::string executable = "/usr/bin/open";
    std::string bundle = targetDir.native();
    char* args[] = {executable.data(), bundle.data(), nullptr};
#else
    std::string executable = getApplicationExecutablePath(targetDir).native();
    char* args[] = {executable.data(), nullptr};
#endif
    pid_t child = 0;
    const int error = posix_spawn(&child, executable.c_str(), nullptr, nullptr, args, environ);
    if (error != 0) {
        UPDATER_ERR("Cannot launch application: " << std::strerror(error));
        return false;
    }

#if defined(__APPLE__)
    // Wait for the open command to finish submitting the launch request.
    int status = 0;
    pid_t waited = 0;
    do {
        waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited != child) {
        const int waitError = errno;
        UPDATER_ERR("Cannot wait for open: " << std::strerror(waitError));
        return false;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        UPDATER_ERR("open did not accept the application launch request");
        return false;
    }
#endif
    return true;
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
