#pragma once

/// @file UpgradeContext.h
/// @brief Shared context data borrowed by the Upgrade FSM.

#include <ucf/services/UpgradeService/UpgradeModel.h>
#include "../UpgradeOperation.h"
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
    std::function<void(UpgradeOperation operation, bool userTriggered)> triggerCheckForUpgrade;
    std::function<void(UpgradeOperation operation, const model::PackageInfo& package)> triggerDownload;
    std::function<void(UpgradeOperation operation, const std::string& filePath, const model::PackageInfo& package)>
        triggerVerify;
    std::function<void(const std::string& installerPath)> triggerOpenInstaller;
    std::function<bool(const std::string& installerPath)> releaseInstaller;

    // ── Sub-manager reset triggers ──
    // hardReset:  clear check result and incomplete downloads; retain verified installer
    // softReset:  clear check result but preserve partial download for resume
    std::function<void()> triggerHardReset;
    std::function<void()> triggerSoftReset;

    // ── Shared data ──
    std::optional<model::UpgradeInfo> availableUpgrade;
    std::string downloadedFilePath;

    /// Operation state is accessed under the FSM mutex; workers receive copies.
    [[nodiscard]] UpgradeOperation beginOperation()
    {
        endOperation();
        ++mLastOperationId;
        mActiveOperationSource.emplace();
        return {mLastOperationId, mActiveOperationSource->get_token()};
    }

    void endOperation() noexcept
    {
        if (mActiveOperationSource)
        {
            mActiveOperationSource->request_stop();
            mActiveOperationSource.reset();
        }
    }

    [[nodiscard]] bool isCurrent(std::uint64_t operationId) const noexcept
    {
        return mActiveOperationSource && operationId == mLastOperationId && !mActiveOperationSource->stop_requested();
    }

    // Called by transition handlers under the FSM mutex.
    void hardReset()
    {
        endOperation();
        availableUpgrade.reset();
        downloadedFilePath.clear();
        triggerHardReset();
    }

    void softReset()
    {
        endOperation();
        triggerSoftReset();
    }

private:
    // This counter also identifies the active operation while its source exists.
    std::uint64_t mLastOperationId{0};
    std::optional<std::stop_source> mActiveOperationSource;
};

} // namespace ucf::service::upgrade
