#pragma once

/// @file UpdaterLog.h
/// @brief Lightweight header-only logging for the updater.
///        Writes to both stdout/stderr and a log file in the temp directory.

#include <filesystem>
#include <fstream>
#include <iostream>

namespace updater::detail {

inline std::ofstream& logFile()
{
    static std::ofstream file(
        (std::filesystem::temp_directory_path() / "template-factory-upgrade" / "updater.log").string(),
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
