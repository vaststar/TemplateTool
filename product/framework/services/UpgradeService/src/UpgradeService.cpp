#include "UpgradeService.h"
#include "UpgradeServiceLogger.h"
#include "UpgradeManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class UpgradeService::DataPrivate
{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;

    UpgradeManager& getUpgradeManager();
    const UpgradeManager& getUpgradeManager() const;

private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    const std::unique_ptr<UpgradeManager> mUpgradeManagerPtr;
};

UpgradeService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mUpgradeManagerPtr(std::make_unique<UpgradeManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr UpgradeService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

UpgradeManager& UpgradeService::DataPrivate::getUpgradeManager()
{
    return *mUpgradeManagerPtr;
}

const UpgradeManager& UpgradeService::DataPrivate::getUpgradeManager() const
{
    return *mUpgradeManagerPtr;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start UpgradeService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IUpgradeService> IUpgradeService::createInstance(
    ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<UpgradeService>(coreFramework);
}

UpgradeService::UpgradeService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    UPGRADE_LOG_DEBUG("Create UpgradeService, address:" << this);
}

UpgradeService::~UpgradeService()
{
    UPGRADE_LOG_DEBUG("Delete UpgradeService, address:" << this);
}

void UpgradeService::initService()
{
    UPGRADE_LOG_INFO("UpgradeService::initService()");
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock()) {
        coreFramework->registerCallback(shared_from_this());
    }
    // Inject this (as Listener) into the manager
    mDataPrivate->getUpgradeManager().initialize(this);
}

std::string UpgradeService::getServiceName() const
{
    return "UpgradeService";
}

void UpgradeService::onServiceInitialized()
{
    UPGRADE_LOG_DEBUG("UpgradeService::onServiceInitialized()");
}

void UpgradeService::onCoreFrameworkExit()
{
    UPGRADE_LOG_DEBUG("UpgradeService::onCoreFrameworkExit()");
}

// ── IUpgradeService — one-line delegation ──

void UpgradeService::checkForUpgrade(bool userTriggered)
{
    mDataPrivate->getUpgradeManager().checkForUpgrade(userTriggered);
}

void UpgradeService::downloadUpgrade()
{
    mDataPrivate->getUpgradeManager().downloadUpgrade();
}

void UpgradeService::installAndRestart()
{
    mDataPrivate->getUpgradeManager().installAndRestart();
}

void UpgradeService::cancelDownload()
{
    mDataPrivate->getUpgradeManager().cancelDownload();
}

void UpgradeService::remindLater()
{
    mDataPrivate->getUpgradeManager().remindLater();
}

model::UpgradeState UpgradeService::getUpgradeState() const
{
    return mDataPrivate->getUpgradeManager().getUpgradeState();
}

std::optional<model::UpgradeInfo> UpgradeService::getAvailableUpgrade() const
{
    return mDataPrivate->getUpgradeManager().getAvailableUpgrade();
}

// ── UpgradeManager::Listener → fireNotification ──

void UpgradeService::onUpgradeStateChanged(model::UpgradeState state)
{
    fireNotification(&IUpgradeServiceCallback::onUpgradeStateChanged, state);
}

void UpgradeService::onUpgradeCheckCompleted(const model::UpgradeCheckResult& result)
{
    fireNotification(&IUpgradeServiceCallback::onUpgradeCheckCompleted, result);
}

void UpgradeService::onDownloadProgressChanged(int64_t currentBytes, int64_t totalBytes)
{
    fireNotification(&IUpgradeServiceCallback::onDownloadProgressChanged, currentBytes, totalBytes);
}

void UpgradeService::onUpgradeError(model::UpgradeErrorCode code, const std::string& msg)
{
    fireNotification(&IUpgradeServiceCallback::onUpgradeError, code, msg);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish UpgradeService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
