#pragma once

/// @file UpgradeExecutor.h
/// @brief Platform-independent upgrade orchestration logic.

#include "../config/UpdaterConfig.h"

namespace updater {

enum class UpgradeExitCode : int {
    Success = 0,
    InvalidInput = 1,              // Invalid PID, paths or staging entry.
    BackupPreparationFailed = 2,  // Current layout, CWD, old backup or backup rename.
    ReplacementFailed = 3,        // Staging rename failed; consult rollback logs.
    InstalledLayoutInvalid = 4,
    ParentExitUnconfirmed = 5,     // Inspection failure or timeout.
    ApplicationLaunchFailed = 6,
    UnexpectedError = 7,
};

/// Execute the full upgrade workflow:
///   validate staging → prepare permissions → wait for parent → validate current
///   → backup → replace → validate installed → launch → clear marker, retain backup
/// Returns a named result; main converts it to the process exit code.
UpgradeExitCode performUpgrade(const UpdaterConfig& config);

} // namespace updater
