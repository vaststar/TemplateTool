#pragma once

/// @file PlatformUtils.h
/// @brief Platform-specific operations for the updater.
///        Implementations are in PlatformUtils_unix.cpp / PlatformUtils_win.cpp.

#include <filesystem>
#include <string>

namespace updater::platform {

/// Wait for a positive process ID to exit (up to timeoutSec seconds).
/// Returns false for an invalid PID, inspection failure or timeout.
bool waitForProcessExit(int pid, int timeoutSec = 30);

/// Extract a ZIP package to destDir using system commands.
/// Returns 0 on success, non-zero on failure.
int extractZipPackage(const std::string& zipPath, const std::string& destDir);

/// Expected main executable inside an installation or staging directory.
std::filesystem::path getApplicationExecutablePath(const std::filesystem::path& root);

/// Submit an application launch request from targetDir.
/// macOS uses `open`, Windows uses CreateProcess, Linux uses posix_spawn.
bool launchApplication(const std::filesystem::path& targetDir);

/// Fix file permissions after extraction (no-op on Windows).
void fixPermissions(const std::filesystem::path& targetDir);

} // namespace updater::platform
