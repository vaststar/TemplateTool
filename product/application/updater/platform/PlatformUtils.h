#pragma once

/// @file PlatformUtils.h
/// @brief Platform-specific operations for the updater.
///        Implementations are in PlatformUtils_unix.cpp / PlatformUtils_win.cpp.

#include <filesystem>
#include <string>

namespace updater::platform {

/// Wait for a process to exit (up to timeoutSec seconds).
/// Returns true if the process has exited, false on timeout.
bool waitForProcessExit(int pid, int timeoutSec = 30);

/// Extract a ZIP package to destDir using system commands.
/// Returns 0 on success, non-zero on failure.
int extractZipPackage(const std::string& zipPath, const std::string& destDir);

/// Launch the application from targetDir.
/// Platform-specific: macOS uses `open`, Windows uses CreateProcess, Linux uses fork+exec.
bool launchApplication(const std::filesystem::path& targetDir);

/// Fix file permissions after extraction (no-op on Windows).
void fixPermissions(const std::filesystem::path& targetDir);

} // namespace updater::platform
