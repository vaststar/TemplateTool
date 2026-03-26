#include "FeatureSettingsService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>

#include "FeatureSettingsManager.h"
#include "FeatureSettingsServiceLogger.h"

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class FeatureSettingsService::DataPrivate {
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    FeatureSettingsManager& getFeatureSettingsManager();
    const FeatureSettingsManager& getFeatureSettingsManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::unique_ptr<FeatureSettingsManager> mFeatureSettingsManager;
};

FeatureSettingsService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mFeatureSettingsManager(std::make_unique<FeatureSettingsManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr FeatureSettingsService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

FeatureSettingsManager& FeatureSettingsService::DataPrivate::getFeatureSettingsManager()
{
    return *mFeatureSettingsManager;
}

const FeatureSettingsManager& FeatureSettingsService::DataPrivate::getFeatureSettingsManager() const
{
    return *mFeatureSettingsManager;
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start FeatureSettingsService Logic////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IFeatureSettingsService> IFeatureSettingsService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<FeatureSettingsService>(coreFramework);
}

FeatureSettingsService::FeatureSettingsService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create FeatureSettingsService, address:" << this);
}

FeatureSettingsService::~FeatureSettingsService()
{
    SERVICE_LOG_DEBUG("Delete FeatureSettingsService, address:" << this);
}

void FeatureSettingsService::initService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
        if (auto dataWarehouseService = coreFramework->getService<ucf::service::IDataWarehouseService>().lock())
        {
            dataWarehouseService->registerCallback(shared_from_this());
        }
    }
}

std::string FeatureSettingsService::getServiceName() const
{
    return "FeatureSettingsService";
}

void FeatureSettingsService::onServiceInitialized()
{
    SERVICE_LOG_DEBUG("FeatureSettingsService initialized");
}

void FeatureSettingsService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("FeatureSettingsService exiting");
}

void FeatureSettingsService::OnDatabaseInitialized(const std::string& dbId)
{
    mDataPrivate->getFeatureSettingsManager().databaseInitialized(dbId);
    fireNotification(&IFeatureSettingsServiceCallback::onFeatureSettingsReady);
}

model::ScreenshotFeatureSettings FeatureSettingsService::getScreenshotSettings() const
{
    return mDataPrivate->getFeatureSettingsManager().getScreenshotSettings();
}

void FeatureSettingsService::updateScreenshotSettings(const model::ScreenshotFeatureSettings& screenshotSettings)
{
    mDataPrivate->getFeatureSettingsManager().updateScreenshotSettings(screenshotSettings);
    fireNotification(&IFeatureSettingsServiceCallback::onScreenshotSettingsChanged, screenshotSettings);
}

model::RecordingFeatureSettings FeatureSettingsService::getRecordingSettings() const
{
    return mDataPrivate->getFeatureSettingsManager().getRecordingSettings();
}

void FeatureSettingsService::updateRecordingSettings(const model::RecordingFeatureSettings& recordingSettings)
{
    mDataPrivate->getFeatureSettingsManager().updateRecordingSettings(recordingSettings);
    fireNotification(&IFeatureSettingsServiceCallback::onRecordingSettingsChanged, recordingSettings);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish FeatureSettingsService Logic///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
