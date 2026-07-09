#include "MiniAppRuntimeViewModel.h"

#include <ucf/Agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentFactory.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/MiniAppService/IMiniAppService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

#include "SystemBridgeHandler.h"

namespace commonHead::viewModels{

namespace {
std::string currentPlatform()
{
#if defined(__APPLE__)
    return "macos";
#elif defined(_WIN32)
    return "windows";
#elif defined(__linux__)
    return "linux";
#else
    return "unknown";
#endif
}
} // namespace

std::shared_ptr<IMiniAppRuntimeViewModel> IMiniAppRuntimeViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MiniAppRuntimeViewModel>(commonHeadFramework);
}

MiniAppRuntimeViewModel::MiniAppRuntimeViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMiniAppRuntimeViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create MiniAppRuntimeViewModel");
}

MiniAppRuntimeViewModel::~MiniAppRuntimeViewModel()
{
    stop();
}

std::string MiniAppRuntimeViewModel::getViewModelName() const
{
    return "MiniAppRuntimeViewModel";
}

void MiniAppRuntimeViewModel::init()
{
    // The runtime is created lazily in start(); nothing to do at construction.
}

std::shared_ptr<ucf::service::IMiniAppService> MiniAppRuntimeViewModel::lockMiniAppService() const
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

std::shared_ptr<ucf::service::IClientInfoService> MiniAppRuntimeViewModel::lockClientInfoService() const
{
    if (auto framework = getCommonHeadFramework().lock())
    {
        if (auto locator = framework->getServiceLocator())
        {
            return locator->getClientInfoService().lock();
        }
    }
    return nullptr;
}

bool MiniAppRuntimeViewModel::start(const std::string& appId)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mAgent)
    {
        COMMONHEAD_LOG_WARN("MiniAppRuntimeViewModel::start already started id=" << mAppId);
        return true;
    }

    auto service = lockMiniAppService();
    if (!service)
    {
        COMMONHEAD_LOG_ERROR("MiniAppRuntimeViewModel::start mini-app service not available");
        return false;
    }

    const auto manifest = service->getApp(appId);
    if (!manifest.has_value())
    {
        COMMONHEAD_LOG_ERROR("MiniAppRuntimeViewModel::start unknown app id=" << appId);
        return false;
    }

    ucf::agents::MiniAppRuntimeAgentConfig config;
    config.appId      = manifest->id;
    config.packageDir = service->getAppPackageDir(manifest->id);
    // Per-app writable cache dir doubles as the WebView2 user-data folder on
    // Windows (isolates each mini-app's cookies/cache). Ignored on macOS.
    config.userDataFolder = service->getAppCacheDir(manifest->id);
    if (!manifest->entry.empty())
    {
        config.entry = manifest->entry;
    }
    config.grantedPermissions = manifest->permissions;

    // Build the built-in system handler content from the view model layer so the
    // handler itself carries no platform/Qt dependency.
    SystemBridgeHandler::Info systemInfo;
    systemInfo.appId    = manifest->id;
    systemInfo.appName  = manifest->name;
    systemInfo.platform = currentPlatform();
    if (auto clientInfo = lockClientInfoService())
    {
        systemInfo.appVersion = clientInfo->getApplicationVersion().toString();
    }

    auto agent = ucf::agents::createMiniAppRuntimeAgent();
    if (!agent)
    {
        COMMONHEAD_LOG_ERROR("MiniAppRuntimeViewModel::start failed to create runtime agent");
        return false;
    }

    agent->registerCallback(
        std::static_pointer_cast<ucf::agents::IMiniAppRuntimeAgentCallback>(shared_from_this()));
    agent->registerBridgeHandler(std::make_shared<SystemBridgeHandler>(std::move(systemInfo)));

    if (!agent->initialize(config))
    {
        COMMONHEAD_LOG_ERROR("MiniAppRuntimeViewModel::start initialize failed id=" << appId);
        return false;
    }

    mAgent  = std::move(agent);
    mAppId  = manifest->id;

    COMMONHEAD_LOG_INFO("MiniAppRuntimeViewModel::start id=" << mAppId << " entry=" << config.entry);
    mAgent->loadEntry();
    return true;
}

void MiniAppRuntimeViewModel::stop()
{
    std::shared_ptr<ucf::agents::IMiniAppRuntimeAgent> agent;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        agent = std::move(mAgent);
        mAgent.reset();
    }
    if (agent)
    {
        agent->shutdown();
    }
}

bool MiniAppRuntimeViewModel::isReady() const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mAgent ? mAgent->isReady() : false;
}

std::uintptr_t MiniAppRuntimeViewModel::nativeHostHandle() const
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mAgent)
    {
        return 0;
    }
    return static_cast<std::uintptr_t>(mAgent->nativeHostHandle());
}

void MiniAppRuntimeViewModel::onReadyChanged(bool ready)
{
    fireNotification(&IMiniAppRuntimeViewModelCallback::onReadyChanged, ready);
}

void MiniAppRuntimeViewModel::onLoadFinished(bool success)
{
    fireNotification(&IMiniAppRuntimeViewModelCallback::onLoadFinished, success);
}

void MiniAppRuntimeViewModel::onLoadFailed(int errorCode, const std::string& errorMessage)
{
    fireNotification(&IMiniAppRuntimeViewModelCallback::onLoadFailed, errorCode, errorMessage);
}

void MiniAppRuntimeViewModel::onTitleChanged(const std::string& title)
{
    fireNotification(&IMiniAppRuntimeViewModelCallback::onTitleChanged, title);
}

void MiniAppRuntimeViewModel::onUrlChanged(const std::string& url)
{
    fireNotification(&IMiniAppRuntimeViewModelCallback::onUrlChanged, url);
}
}
