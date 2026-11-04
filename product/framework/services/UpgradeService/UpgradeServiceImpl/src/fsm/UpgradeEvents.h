#pragma once

/// @file UpgradeEvents.h
/// @brief Event types for the Upgrade FSM.

#include <ucf/services/UpgradeService/UpgradeModel.h>
#include <cstdint>
#include <optional>
#include <string>

namespace ucf::service::upgrade {

// ── User / timer triggered events ──

struct EvCheckRequested
{
    bool userTriggered{false};
};
struct EvDownloadStart
{
};
struct EvOpenInstaller
{
};
struct EvCancel
{
};
/// User dismissed the current upgrade prompt or error — return to Idle.
/// Handled by UpgradeAvailable, ReadyToInstall, InstallerOpened and Failed.
struct EvDismiss
{
};

// ── Async result events (posted from manager callbacks) ──

/// Manifest check and local cache lookup have both completed. A cached candidate
/// must still pass verification before it can be opened.
struct EvCheckSuccess
{
    std::uint64_t operationId;
    model::UpgradeInfo info;
    std::optional<std::string> cachedFilePath{};
};
struct EvCheckNoUpgrade
{
    std::uint64_t operationId;
};
struct EvProgress
{
    std::uint64_t operationId;
    int64_t current{0};
    int64_t total{0};
};
struct EvDownloadDone
{
    std::uint64_t operationId;
    std::string filePath;
};
struct EvVerifyOk
{
    std::uint64_t operationId;
};
struct EvInstallerOpenResult
{
    bool success{false};
};
struct EvError
{
    std::uint64_t operationId;
    model::UpgradeErrorCode code;
    std::string message;
};

} // namespace ucf::service::upgrade
