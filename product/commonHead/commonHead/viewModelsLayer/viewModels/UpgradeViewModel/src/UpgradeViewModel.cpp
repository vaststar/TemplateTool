#include "UpgradeViewModel.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <ucf/Services/UpgradeService/IUpgradeService.h>

namespace commonHead::viewModels{

std::shared_ptr<IUpgradeViewModel> IUpgradeViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<UpgradeViewModel>(commonHeadFramework);
}

UpgradeViewModel::UpgradeViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IUpgradeViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create UpgradeViewModel");
}

std::string UpgradeViewModel::getViewModelName() const
{
    return "UpgradeViewModel";
}

void UpgradeViewModel::init()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto service = serviceLocator->getUpgradeService().lock())
            {
                service->registerCallback(shared_from_this());
            }
        }
    }
}

// ── Actions → delegate to IUpgradeService ──

void UpgradeViewModel::checkForUpgrade()
{
    COMMONHEAD_LOG_DEBUG("checkForUpgrade (user triggered)");
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto service = serviceLocator->getUpgradeService().lock())
            {
                service->checkForUpgrade(/*userTriggered=*/true);
            }
        }
    }
}

void UpgradeViewModel::downloadUpgrade()
{
    COMMONHEAD_LOG_DEBUG("downloadUpgrade");
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto service = serviceLocator->getUpgradeService().lock())
            {
                service->downloadUpgrade();
            }
        }
    }
}

void UpgradeViewModel::installAndRestart()
{
    COMMONHEAD_LOG_DEBUG("installAndRestart");
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto service = serviceLocator->getUpgradeService().lock())
            {
                service->installAndRestart();
            }
        }
    }
}

void UpgradeViewModel::cancelDownload()
{
    COMMONHEAD_LOG_DEBUG("cancelDownload");
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto service = serviceLocator->getUpgradeService().lock())
            {
                service->cancelDownload();
            }
        }
    }
}

void UpgradeViewModel::dismissUpgrade()
{
    COMMONHEAD_LOG_DEBUG("dismissUpgrade");
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto service = serviceLocator->getUpgradeService().lock())
            {
                service->dismissUpgrade();
            }
        }
    }
}

// ── IUpgradeServiceCallback → forward to UI via fireNotification ──

void UpgradeViewModel::onUpgradeStateChanged(ucf::service::model::UpgradeState state)
{
    using S = ucf::service::model::UpgradeState;
    using V = model::UpgradeViewState;
    static const std::pair<S, V> mapping[] = {
        {S::Idle,             V::Idle},
        {S::Checking,         V::Checking},
        {S::UpgradeAvailable, V::UpgradeAvailable},
        {S::Downloading,      V::Downloading},
        {S::Verifying,        V::Verifying},
        {S::Extracting,       V::Extracting},
        {S::ReadyToInstall,   V::ReadyToInstall},
        {S::Installing,       V::Installing},
        {S::Failed,           V::Failed},
    };
    auto viewState = V::Idle;
    for (const auto& [s, v] : mapping) {
        if (s == state) { viewState = v; break; }
    }
    fireNotification(&IUpgradeViewModelCallback::onUpgradeStateChanged, viewState);
}

void UpgradeViewModel::onUpgradeCheckCompleted(const ucf::service::model::UpgradeCheckResult& result)
{
    model::UpgradeViewInfo info;
    if (result.hasUpgrade) {
        info.version = result.upgradeInfo.version;
        info.releaseDate = result.upgradeInfo.releaseDate;
        info.releaseNotes = result.upgradeInfo.releaseNotes;
        info.mandatory = result.upgradeInfo.mandatory;
    }
    fireNotification(&IUpgradeViewModelCallback::onCheckCompleted, result.hasUpgrade, info);
}

void UpgradeViewModel::onDownloadProgressChanged(int64_t currentBytes, int64_t totalBytes)
{
    fireNotification(&IUpgradeViewModelCallback::onDownloadProgress, currentBytes, totalBytes);
}

void UpgradeViewModel::onUpgradeError(ucf::service::model::UpgradeErrorCode /*errorCode*/, const std::string& message)
{
    fireNotification(&IUpgradeViewModelCallback::onUpgradeError, message);
}

} // namespace commonHead::viewModels
