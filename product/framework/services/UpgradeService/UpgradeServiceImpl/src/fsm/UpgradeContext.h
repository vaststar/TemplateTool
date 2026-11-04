#pragma once

/// @file UpgradeContext.h
/// @brief Shared context data borrowed by the Upgrade FSM.

#include <ucf/services/UpgradeService/UpgradeModel.h>
#include <functional>
#include <optional>
#include <string>

namespace ucf::service::upgrade {

struct UpgradeContext
{
    // ── Notification callbacks (Service → UI) ──
    std::function<void(model::UpgradeState)> onStateChanged;
    std::function<void(const model::UpgradeCheckResult&)> onCheckCompleted;
    std::function<void(int64_t, int64_t)> onDownloadProgress;
    std::function<void(model::UpgradeErrorCode, const std::string&)> onError;

    // ── Async operation triggers (bound to Manager methods) ──
    // Checking covers the manifest request and local installer cache lookup.
    std::function<void(bool userTriggered)> triggerCheckForUpgrade;
    std::function<void(const std::string& url)> triggerDownload;
    std::function<void(const std::string& path)> triggerVerify;
    std::function<void(const std::string& installerPath)> triggerOpenInstaller;
    std::function<bool(const std::string& installerPath)> releaseInstaller;

    // ── Reset triggers ──
    // hardReset:  clear check result and incomplete downloads; retain verified installer
    // softReset:  clear caches but PRESERVE partial download (so resume is possible)
    std::function<void()> triggerHardReset;
    std::function<void()> triggerSoftReset;

    // ── Shared data ──
    std::optional<model::UpgradeInfo> availableUpgrade;
    std::string downloadedFilePath;
};

} // namespace ucf::service::upgrade
