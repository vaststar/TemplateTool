#include "MiniAppService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include "MiniAppManager.h"
#include "MiniAppServiceLogger.h"

namespace ucf::service {

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class MiniAppService::DataPrivate {
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    MiniAppManager& getMiniAppManager();
    const MiniAppManager& getMiniAppManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::unique_ptr<MiniAppManager> mMiniAppManager;
};

MiniAppService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mMiniAppManager(std::make_unique<MiniAppManager>(coreFramework))
{
}

ucf::framework::ICoreFrameworkWPtr MiniAppService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

MiniAppManager& MiniAppService::DataPrivate::getMiniAppManager()
{
    return *mMiniAppManager;
}

const MiniAppManager& MiniAppService::DataPrivate::getMiniAppManager() const
{
    return *mMiniAppManager;
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start MiniAppService Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IMiniAppService> IMiniAppService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<MiniAppService>(coreFramework);
}

MiniAppService::MiniAppService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create MiniAppService, address:" << this);
}

MiniAppService::~MiniAppService()
{
    SERVICE_LOG_DEBUG("Delete MiniAppService, address:" << this);
}

void MiniAppService::initService()
{
    SERVICE_LOG_DEBUG("MiniAppService::initService()");
    // Inject the upward sink before any scan so no early events are lost.
    mDataPrivate->getMiniAppManager().setNotificationSink(
        std::static_pointer_cast<IMiniAppNotificationSink>(shared_from_this()));

    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }
}

void MiniAppService::deinitService()
{
    SERVICE_LOG_DEBUG("MiniAppService::deinitService()");
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->unRegisterCallback(shared_from_this());
    }
}

std::string MiniAppService::getServiceName() const
{
    return "MiniAppService";
}

std::vector<ServiceDependency> MiniAppService::dependencies() const
{
    return {
        { std::type_index(typeid(IClientInfoService)), DependencyKind::Required }
    };
}

void MiniAppService::onServiceInitialized()
{
    SERVICE_LOG_DEBUG("MiniAppService initialized");
    // ClientInfoService (path roots) is guaranteed ready once all services are
    // initialized, so perform the first scan of installed packages here. The
    // Manager notifies us back via the sink (onMiniAppServiceReady).
    mDataPrivate->getMiniAppManager().rescan();
}

void MiniAppService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("MiniAppService exiting");
}

bool MiniAppService::isReady() const
{
    const bool ready = mDataPrivate->getMiniAppManager().isReady();
    SERVICE_LOG_DEBUG("isReady: " << (ready ? "true" : "false"));
    return ready;
}

std::vector<model::MiniAppManifest> MiniAppService::listInstalledApps() const
{
    auto apps = mDataPrivate->getMiniAppManager().listInstalledApps();
    SERVICE_LOG_DEBUG("listInstalledApps: " << apps.size() << " mini-app(s)");
    return apps;
}

std::optional<model::MiniAppManifest> MiniAppService::getApp(const std::string& id) const
{
    auto app = mDataPrivate->getMiniAppManager().getApp(id);
    if (app)
    {
        SERVICE_LOG_DEBUG("getApp: found '" << app->name << "' (id:" << id << ")");
    }
    else
    {
        SERVICE_LOG_DEBUG("getApp: not found, id:" << id);
    }
    return app;
}

bool MiniAppService::installFromDirectory(const std::string& sourceDirectory)
{
    SERVICE_LOG_INFO("installFromDirectory: " << sourceDirectory);
    // The Manager updates state and fires onMiniAppInstalled via the sink.
    return mDataPrivate->getMiniAppManager().installFromDirectory(sourceDirectory).has_value();
}

bool MiniAppService::uninstall(const std::string& id)
{
    SERVICE_LOG_INFO("uninstall, id:" << id);
    // The Manager updates state and fires onMiniAppUninstalled via the sink.
    return mDataPrivate->getMiniAppManager().uninstall(id);
}

std::string MiniAppService::getAppPackageDir(const std::string& id) const
{
    SERVICE_LOG_DEBUG("getAppPackageDir, id:" << id);
    return mDataPrivate->getMiniAppManager().getAppPackageDir(id);
}

std::string MiniAppService::getAppStorageDir(const std::string& id) const
{
    SERVICE_LOG_DEBUG("getAppStorageDir, id:" << id);
    return mDataPrivate->getMiniAppManager().getAppStorageDir(id);
}

std::string MiniAppService::getAppCacheDir(const std::string& id) const
{
    SERVICE_LOG_DEBUG("getAppCacheDir, id:" << id);
    return mDataPrivate->getMiniAppManager().getAppCacheDir(id);
}

std::string MiniAppService::getAppIconPath(const std::string& id) const
{
    SERVICE_LOG_DEBUG("getAppIconPath, id:" << id);
    return mDataPrivate->getMiniAppManager().getAppIconPath(id);
}

// ===== IMiniAppNotificationSink — translate internal events to outward callbacks =====
void MiniAppService::onMiniAppServiceReady()
{
    SERVICE_LOG_DEBUG("fire onMiniAppServiceReady");
    fireNotification(&IMiniAppServiceCallback::onMiniAppServiceReady);
}

void MiniAppService::onMiniAppInstalled(const model::MiniAppManifest& app)
{
    SERVICE_LOG_INFO("fire onMiniAppInstalled, name:'" << app.name << "' (id:" << app.id << ")");
    fireNotification(&IMiniAppServiceCallback::onMiniAppInstalled, app);
}

void MiniAppService::onMiniAppUninstalled(const std::string& id)
{
    SERVICE_LOG_INFO("fire onMiniAppUninstalled, id:" << id);
    fireNotification(&IMiniAppServiceCallback::onMiniAppUninstalled, id);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish MiniAppService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

} // namespace ucf::service
