#pragma once

/// @file UpgradeContext.h
/// @brief Shared context data borrowed by the Upgrade FSM.

#include <ucf/Services/UpgradeService/UpgradeModel.h>
#include <functional>
#include <optional>
#include <string>

namespace ucf::service::upgrade {

struct UpgradeContext {
    // ── Notification callbacks (Service → UI) ──
    std::function<void(model::UpgradeState)>                              onStateChanged;
    std::function<void(const model::UpgradeCheckResult&)>                 onCheckCompleted;
    std::function<void(int64_t, int64_t)>                                 onDownloadProgress;
    std::function<void(model::UpgradeErrorCode, const std::string&)>      onError;

    // ── Async operation triggers (bound to Manager methods) ──
    std::function<void(bool userTriggered)>      triggerCheckForUpgrade;
    std::function<void(const std::string& url)>  triggerDownload;
    std::function<void(const std::string& path)> triggerVerify;
    std::function<void(const std::string& path)> triggerInstall;
    std::function<void()>                        triggerCancelDownload;

    // ── Reset triggers ──
    std::function<void()>                        triggerResetManagers;
    std::function<void()>                        triggerSoftResetManagers;

    // ── Shared data ──
    std::optional<model::UpgradeInfo> availableUpgrade;
    std::string downloadedFilePath;
    bool keepPartialDownload{false};
};

} // namespace ucf::service::upgrade
