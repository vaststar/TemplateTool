#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/FeatureSettingsService/IFeatureSettingsService.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h>

namespace ucf::service {

class SERVICE_EXPORT FeatureSettingsService final
    : public virtual IFeatureSettingsService
    , public virtual ucf::utilities::NotificationHelper<IFeatureSettingsServiceCallback>
    , public ucf::framework::CoreFrameworkCallbackDefault
    , public ucf::service::IDataWarehouseServiceCallback
    , public std::enable_shared_from_this<FeatureSettingsService>
{
public:
    explicit FeatureSettingsService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~FeatureSettingsService();
    FeatureSettingsService(const FeatureSettingsService&) = delete;
    FeatureSettingsService(FeatureSettingsService&&) = delete;
    FeatureSettingsService& operator=(const FeatureSettingsService&) = delete;
    FeatureSettingsService& operator=(FeatureSettingsService&&) = delete;

public:
    // IService
    virtual std::string getServiceName() const override;

    // ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;

    // IDataWarehouseServiceCallback
    virtual void OnDatabaseInitialized(const std::string& dbId) override;

    // IFeatureSettingsService
    virtual model::ScreenshotFeatureSettings getScreenshotSettings() const override;
    virtual void updateScreenshotSettings(const model::ScreenshotFeatureSettings& screenshotSettings) override;

protected:
    // IService
    virtual void initService() override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
