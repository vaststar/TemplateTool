#pragma once

/// @file UpdaterLog.h
/// @brief Lightweight header-only logging macros for the updater.
///        No library dependencies — just stdout/stderr with a prefix.

#include <iostream>

#define UPDATER_LOG(msg)  std::cout << "[updater] " << msg << std::endl
#define UPDATER_ERR(msg)  std::cerr << "[updater] ERROR: " << msg << std::endl
#define UPDATER_WARN(msg) std::cerr << "[updater] Warning: " << msg << std::endl
