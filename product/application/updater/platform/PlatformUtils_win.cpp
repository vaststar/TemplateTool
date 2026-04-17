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
        return true;
    }

    UPDATER_LOG("Waiting for process " << pid << " to exit...");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);

    while (std::chrono::steady_clock::now() < deadline) {
        HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
        if (!hProcess) {
            return true; // Process doesn't exist
        }
        DWORD result = WaitForSingleObject(hProcess, 500);
        CloseHandle(hProcess);
        if (result == WAIT_OBJECT_0) {
            return true;
        }
    }

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

bool launchApplication(const std::filesystem::path& targetDir)
{
    auto exePath = targetDir / "mainEntry.exe";
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
    return false;
}

void fixPermissions(const std::filesystem::path& /*targetDir*/)
{
    // No-op on Windows — executables don't need permission fixes
}

} // namespace updater::platform

#endif // _WIN32
