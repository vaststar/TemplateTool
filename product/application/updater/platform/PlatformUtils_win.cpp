#ifdef _WIN32

#include "PlatformUtils.h"
#include "../core/UpdaterLog.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <thread>

#include <windows.h>
#include <tlhelp32.h>

namespace updater::platform {

bool waitForProcessExit(int pid, int timeoutSec)
{
    if (pid <= 0) {
        UPDATER_ERR("Invalid parent PID: " << pid);
        return false;
    }

    UPDATER_LOG("Waiting for process " << pid << " to exit...");

    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
    if (!process) {
        const DWORD error = GetLastError();
        if (error == ERROR_INVALID_PARAMETER) {
            return true;
        }
        UPDATER_ERR("OpenProcess failed: " << error);
        return false;
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);
    while (std::chrono::steady_clock::now() < deadline) {
        const DWORD result = WaitForSingleObject(process, 500);
        if (result == WAIT_OBJECT_0) {
            CloseHandle(process);
            return true;
        }
        if (result == WAIT_FAILED) {
            const DWORD error = GetLastError();
            CloseHandle(process);
            UPDATER_ERR("WaitForSingleObject failed: " << error);
            return false;
        }
        if (result != WAIT_TIMEOUT) {
            CloseHandle(process);
            UPDATER_ERR("Unexpected process wait result: " << result);
            return false;
        }
    }
    CloseHandle(process);
    UPDATER_WARN("Timed out waiting for process " << pid);
    return false;
}

int extractZipPackage(const std::string& zipPath, const std::string& destDir)
{
    std::error_code ec;
    std::filesystem::create_directories(destDir, ec);

    // Windows: use PowerShell Expand-Archive
    std::ostringstream cmd;
    cmd << "powershell -NoProfile -Command \""
        << "Expand-Archive -Path '" << zipPath
        << "' -DestinationPath '" << destDir
        << "' -Force\"";

    UPDATER_LOG("Running: " << cmd.str());
    int ret = std::system(cmd.str().c_str());

    if (ret != 0) {
        UPDATER_ERR("Extract command returned " << ret);
    }
    return ret;
}

std::filesystem::path getApplicationExecutablePath(const std::filesystem::path& root)
{
    return root / "bin" / "mainEntry.exe";
}

bool launchApplication(const std::filesystem::path& targetDir)
{
    auto exePath = getApplicationExecutablePath(targetDir);
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    auto wPath = exePath.wstring();
    if (CreateProcessW(wPath.c_str(), nullptr, nullptr, nullptr,
                       FALSE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    const DWORD error = GetLastError();
    UPDATER_ERR("CreateProcess failed: " << error);
    return false;
}

void fixPermissions(const std::filesystem::path& /*targetDir*/)
{
    // No-op on Windows — executables don't need permission fixes
}

} // namespace updater::platform

#endif // _WIN32
