#pragma once

/// @file UpgradeExecutor.h
/// @brief Platform-independent upgrade orchestration logic.

#include "../config/UpdaterConfig.h"

namespace updater {

/// Execute the full upgrade workflow:
///   validate → wait for parent → backup → extract → fix permissions → launch → cleanup
/// Returns 0 on success, non-zero exit code on failure.
int performUpgrade(const UpdaterConfig& config);

} // namespace updater
