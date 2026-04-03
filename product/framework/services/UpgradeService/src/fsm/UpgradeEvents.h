#pragma once

/// @file UpgradeEvents.h
/// @brief Event types for the Upgrade FSM.

#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <cstdint>
#include <string>

namespace ucf::service::upgrade {

// ── User / timer triggered events ──

struct EvCheckRequested   { bool userTriggered{false}; };
struct EvDownloadStart    {};
struct EvInstallStart     {};
struct EvCancel           {};
struct EvRemindLater      {};
struct EvReset            {};

// ── Async result events (posted from manager callbacks) ──

struct EvCheckSuccess     { model::UpgradeInfo info; };
struct EvCheckNoUpgrade   {};
struct EvProgress         { int64_t current{0}; int64_t total{0}; };
struct EvDownloadDone     { std::string filePath; };
struct EvVerifyOk         {};
struct EvError            { model::UpgradeErrorCode code; std::string message; };

} // namespace ucf::service::upgrade
