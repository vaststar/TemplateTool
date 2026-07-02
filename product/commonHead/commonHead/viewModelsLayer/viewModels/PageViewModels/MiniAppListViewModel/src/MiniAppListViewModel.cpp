#include "MiniAppListViewModel.h"

#include <algorithm>

#include <ucf/Services/MiniAppService/IMiniAppService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

namespace commonHead::viewModels{

namespace {
model::MiniAppInfo toMiniAppInfo(const ucf::service::model::MiniAppManifest& manifest,
                                 const std::shared_ptr<ucf::service::IMiniAppService>& service)
{
    std::string iconPath;
    if (service)
    {
        iconPath = service->getAppIconPath(manifest.id);
    }
    return model::MiniAppInfo{ manifest.id, manifest.name, manifest.description, manifest.entry, std::move(iconPath) };
}
} // namespace

std::shared_ptr<IMiniAppListViewModel> IMiniAppListViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MiniAppListViewModel>(commonHeadFramework);
}

MiniAppListViewModel::MiniAppListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMiniAppListViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create MiniAppListViewModel");
}

std::string MiniAppListViewModel::getViewModelName() const
{
    return "MiniAppListViewModel";
}

void MiniAppListViewModel::init()
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("MiniAppListViewModel init: service not available");
        return;
    }
    // Register first so we do not miss the ready event between the probe and the read.
    service->registerCallback(
        std::static_pointer_cast<ucf::service::IMiniAppServiceCallback>(shared_from_this()));

    // If the service already finished its scan, read the snapshot directly and
    // notify subscribers. Otherwise leave the list empty and wait for the
    // onMiniAppServiceReady() callback to populate it.
    if (service->isReady())
    {
        rebuildFromService();
        fireNotification(&IMiniAppListViewModelCallback::onMiniAppListChanged);
    }
}

std::shared_ptr<ucf::service::IMiniAppService> MiniAppListViewModel::lockService() const
{
    if (auto framework = getCommonHeadFramework().lock())
    {
        if (auto locator = framework->getServiceLocator())
        {
            return locator->getMiniAppService().lock();
        }
    }
    return nullptr;
}

void MiniAppListViewModel::rebuildFromService()
{
    auto service = lockService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("MiniAppListViewModel rebuild skipped: service not available");
        return;
    }

    std::vector<model::MiniAppInfo> apps;
    for (const auto& manifest : service->listInstalledApps())
    {
        apps.push_back(toMiniAppInfo(manifest, service));
    }
    COMMONHEAD_LOG_DEBUG("MiniAppListViewModel rebuild: " << apps.size() << " app(s)");
    {
        std::scoped_lock lock(mMutex);
        mMiniApps = std::move(apps);
    }
}

std::vector<commonHead::viewModels::model::MiniAppInfo> MiniAppListViewModel::getMiniApps() const
{
    std::scoped_lock lock(mMutex);
    return mMiniApps;
}

commonHead::viewModels::model::MiniAppInfo MiniAppListViewModel::getMiniApp(const std::string& id) const
{
    std::scoped_lock lock(mMutex);
    const auto it = std::find_if(mMiniApps.begin(), mMiniApps.end(),
        [&id](const auto& app) { return app.id == id; });
    if (it != mMiniApps.end())
    {
        return *it;
    }
    return {};
}

// ===== IMiniAppServiceCallback: apply service delta, then notify subscribers =====
void MiniAppListViewModel::onMiniAppServiceReady()
{
    COMMONHEAD_LOG_DEBUG("onMiniAppServiceReady received from service");
    rebuildFromService();
    fireNotification(&IMiniAppListViewModelCallback::onMiniAppListChanged);
}

void MiniAppListViewModel::onMiniAppInstalled(const ucf::service::model::MiniAppManifest& app)
{
    COMMONHEAD_LOG_DEBUG("onMiniAppInstalled received from service, id:" << app.id);
    auto service = lockService();
    {
        std::scoped_lock lock(mMutex);
        const auto it = std::find_if(mMiniApps.begin(), mMiniApps.end(),
            [&app](const auto& existing) { return existing.id == app.id; });
        if (it == mMiniApps.end())
        {
            mMiniApps.push_back(toMiniAppInfo(app, service));
        }
        else
        {
            *it = toMiniAppInfo(app, service);
        }
    }
    fireNotification(&IMiniAppListViewModelCallback::onMiniAppListChanged);
}

void MiniAppListViewModel::onMiniAppUninstalled(const std::string& id)
{
    COMMONHEAD_LOG_DEBUG("onMiniAppUninstalled received from service, id:" << id);
    {
        std::scoped_lock lock(mMutex);
        std::erase_if(mMiniApps, [&id](const auto& app) { return app.id == id; });
    }
    fireNotification(&IMiniAppListViewModelCallback::onMiniAppListChanged);
}
}
