#pragma once

#include <cstdint>
#include <string>

#include <ucf/Services/UpgradeService/UpgradeModel.h>

namespace ucf::service {

class IUpgradeServiceCallback
{
public:
    IUpgradeServiceCallback() = default;
    IUpgradeServiceCallback(const IUpgradeServiceCallback&) = delete;
    IUpgradeServiceCallback(IUpgradeServiceCallback&&) = delete;
    IUpgradeServiceCallback& operator=(const IUpgradeServiceCallback&) = delete;
    IUpgradeServiceCallback& operator=(IUpgradeServiceCallback&&) = delete;
    virtual ~IUpgradeServiceCallback() = default;

public:
    /// Called when upgrade state changes
    virtual void onUpgradeStateChanged(model::UpgradeState state) = 0;

    /// Called when check-for-upgrade completes
    virtual void onUpgradeCheckCompleted(const model::UpgradeCheckResult& result) = 0;

    /// Called periodically during download (throttled to ~200ms intervals)
    virtual void onDownloadProgressChanged(int64_t currentBytes, int64_t totalBytes) = 0;

    /// Called when an error occurs
    virtual void onUpgradeError(model::UpgradeErrorCode errorCode, const std::string& message) = 0;
};

} // namespace ucf::service
