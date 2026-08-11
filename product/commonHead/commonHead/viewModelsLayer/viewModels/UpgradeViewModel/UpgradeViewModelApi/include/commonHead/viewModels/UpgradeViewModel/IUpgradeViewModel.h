#pragma once

#include <cstdint>
#include <string>

#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

namespace commonHead::viewModels{

namespace model{

enum class UpgradeViewState : uint8_t {
    Idle,
    Checking,
    UpgradeAvailable,
    Downloading,
    Verifying,
    Extracting,
    ReadyToInstall,
    Installing,
    Failed
};

struct UpgradeViewInfo {
    std::string version;
    std::string releaseDate;
    std::string releaseNotes;
    bool mandatory{false};
};
} // namespace model

class IUpgradeViewModelCallback
{
public:
    IUpgradeViewModelCallback() = default;
    IUpgradeViewModelCallback(const IUpgradeViewModelCallback&) = delete;
    IUpgradeViewModelCallback(IUpgradeViewModelCallback&&) = delete;
    IUpgradeViewModelCallback& operator=(const IUpgradeViewModelCallback&) = delete;
    IUpgradeViewModelCallback& operator=(IUpgradeViewModelCallback&&) = delete;
    virtual ~IUpgradeViewModelCallback() = default;
public:
    virtual void onUpgradeStateChanged(model::UpgradeViewState state) = 0;
    virtual void onCheckCompleted(bool hasUpgrade, const model::UpgradeViewInfo& info) = 0;
    virtual void onDownloadProgress(int64_t currentBytes, int64_t totalBytes) = 0;
    virtual void onUpgradeError(const std::string& message) = 0;
};

class IUpgradeViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IUpgradeViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IUpgradeViewModel(const IUpgradeViewModel&) = delete;
    IUpgradeViewModel(IUpgradeViewModel&&) = delete;
    IUpgradeViewModel& operator=(const IUpgradeViewModel&) = delete;
    IUpgradeViewModel& operator=(IUpgradeViewModel&&) = delete;
    virtual ~IUpgradeViewModel() = default;
public:
    virtual void checkForUpgrade() = 0;
    virtual void downloadUpgrade() = 0;
    virtual void installAndRestart() = 0;
    virtual void cancelDownload() = 0;
    virtual void dismissUpgrade() = 0;
};
}
