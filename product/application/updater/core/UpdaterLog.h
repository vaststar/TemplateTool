#pragma once

/// @file UpdaterLog.h
/// @brief Lightweight header-only logging for the updater.
///        Writes to both stdout/stderr and a log file next to the updater binary.

#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#endif

namespace updater::detail {

/// Get the directory containing the running updater binary.
inline std::filesystem::path getUpdaterDirectory()
{
#ifdef _WIN32
    wchar_t buf[MAX_PATH]{};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path();
#elif defined(__APPLE__)
    char buf[4096]{};
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        return std::filesystem::path(buf).parent_path();
    }
    return std::filesystem::temp_directory_path();
#else
    char buf[4096]{};
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        return std::filesystem::path(buf).parent_path();
    }
    return std::filesystem::temp_directory_path();
#endif
}

inline std::ofstream& logFile()
{
    static std::ofstream file(
        (getUpdaterDirectory() / "updater.log").string(),
        std::ios::app);
    return file;
}

} // namespace updater::detail

#define UPDATER_LOG(msg)  do { std::cout << "[updater] " << msg << std::endl; \
    updater::detail::logFile() << "[updater] " << msg << std::endl; } while(0)
#define UPDATER_ERR(msg)  do { std::cerr << "[updater] ERROR: " << msg << std::endl; \
    updater::detail::logFile() << "[updater] ERROR: " << msg << std::endl; } while(0)
#define UPDATER_WARN(msg) do { std::cerr << "[updater] Warning: " << msg << std::endl; \
    updater::detail::logFile() << "[updater] Warning: " << msg << std::endl; } while(0)
