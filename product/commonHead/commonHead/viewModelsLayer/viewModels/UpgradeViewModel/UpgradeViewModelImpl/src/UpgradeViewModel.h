#pragma once

#include <memory>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/UpgradeViewModel/IUpgradeViewModel.h>
#include <ucf/Services/UpgradeService/IUpgradeServiceCallback.h>

namespace commonHead::viewModels{
class UpgradeViewModel: public virtual IUpgradeViewModel,
                        public virtual commonHead::utilities::VMNotificationHelper<IUpgradeViewModelCallback>,
                        public ucf::service::IUpgradeServiceCallback,
                        public std::enable_shared_from_this<UpgradeViewModel>
{
public:
    explicit UpgradeViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    UpgradeViewModel(const UpgradeViewModel&) = delete;
    UpgradeViewModel(UpgradeViewModel&&) = delete;
    UpgradeViewModel& operator=(const UpgradeViewModel&) = delete;
    UpgradeViewModel& operator=(UpgradeViewModel&&) = delete;
    ~UpgradeViewModel() = default;
public:
    std::string getViewModelName() const override;

    // IUpgradeViewModel
    void checkForUpgrade() override;
    void downloadUpgrade() override;
    void installAndRestart() override;
    void cancelDownload() override;
    void dismissUpgrade() override;

    // IUpgradeServiceCallback
    void onUpgradeStateChanged(ucf::service::model::UpgradeState state) override;
    void onUpgradeCheckCompleted(const ucf::service::model::UpgradeCheckResult& result) override;
    void onDownloadProgressChanged(int64_t currentBytes, int64_t totalBytes) override;
    void onUpgradeError(ucf::service::model::UpgradeErrorCode errorCode, const std::string& message) override;

protected:
    void init() override;
};
}
