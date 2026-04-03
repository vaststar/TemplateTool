#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/UpgradeService/IUpgradeService.h>

#include "UpgradeManager.h"

namespace ucf::service {

class SERVICE_EXPORT UpgradeService final
    : public virtual IUpgradeService
    , public virtual ucf::utilities::NotificationHelper<IUpgradeServiceCallback>
    , public ucf::framework::CoreFrameworkCallbackDefault
    , public std::enable_shared_from_this<UpgradeService>
    , private UpgradeManager::Listener  // Receives notifications from Manager
{
public:
    explicit UpgradeService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~UpgradeService();
    UpgradeService(const UpgradeService&) = delete;
    UpgradeService(UpgradeService&&) = delete;
    UpgradeService& operator=(const UpgradeService&) = delete;
    UpgradeService& operator=(UpgradeService&&) = delete;

public:
    // IService
    virtual std::string getServiceName() const override;

    // ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;

    // IUpgradeService — actions
    virtual void checkForUpgrade(bool userTriggered) override;
    virtual void downloadUpgrade() override;
    virtual void installAndRestart() override;
    virtual void cancelDownload() override;
    virtual void remindLater() override;

    // IUpgradeService — queries
    virtual model::UpgradeState getUpgradeState() const override;
    virtual std::optional<model::UpgradeInfo> getAvailableUpgrade() const override;

protected:
    // IService
    virtual void initService() override;

private:
    // UpgradeManager::Listener (private inheritance)
    void onUpgradeStateChanged(model::UpgradeState state) override;
    void onUpgradeCheckCompleted(const model::UpgradeCheckResult& result) override;
    void onDownloadProgressChanged(int64_t currentBytes, int64_t totalBytes) override;
    void onUpgradeError(model::UpgradeErrorCode code, const std::string& msg) override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
