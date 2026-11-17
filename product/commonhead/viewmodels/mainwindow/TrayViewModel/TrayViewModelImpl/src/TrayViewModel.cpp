#include "TrayViewModel.h"
#include "LoggerDefine.h"

#include <algorithm>
#include <utility>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ResourceLoader/IResourceLoader.h>
#include <commonhead/ResourceStringLoader/ResourceString.h>
#include <commonhead/viewmodels/TrayViewModel/TrayViewModelCreator.h>

namespace commonHead::viewModels {

namespace impl {

std::shared_ptr<ITrayViewModel> createTrayViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<TrayViewModel>(commonHeadFramework);
}

} // namespace impl

TrayViewModel::TrayViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : ITrayViewModel(commonHeadFramework)
{
    TRAY_VIEW_MODEL_LOG_DEBUG("TrayViewModel constructed, address: " << this);
}

TrayViewModel::~TrayViewModel()
{
    TRAY_VIEW_MODEL_LOG_DEBUG("TrayViewModel destroying, address: " << this);
}

std::string TrayViewModel::getViewModelName() const
{
    return "TrayViewModel";
}

void TrayViewModel::init()
{
    reloadMenu();
}

std::vector<model::TrayMenuItemData> TrayViewModel::getMenuItems() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_menuItems;
}

void TrayViewModel::reloadMenu()
{
    auto framework = getCommonHeadFramework().lock();
    if (!framework)
    {
        TRAY_VIEW_MODEL_LOG_ERROR("reloadMenu: CommonHeadFramework is not available");
        return;
    }

    auto resourceLoader = framework->getResourceLoader();
    if (!resourceLoader)
    {
        TRAY_VIEW_MODEL_LOG_ERROR("reloadMenu: ResourceLoader is not available");
        return;
    }

    std::vector<model::TrayMenuItemData> menuItems = {
        {
            model::TrayActionId::ShowMainWindow,
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::MenuShowMainWindow),
            model::TrayMenuItemKind::Action,
            true
        },
        { model::TrayActionId::Unknown, {}, model::TrayMenuItemKind::Separator, false },
        {
            model::TrayActionId::OpenSettings,
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavSettings),
            model::TrayMenuItemKind::Action,
            true
        },
        {
            model::TrayActionId::CheckForUpdates,
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::MenuCheckUpgrade),
            model::TrayMenuItemKind::Action,
            true
        },
        {
            model::TrayActionId::ShowAbout,
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::MenuAbout),
            model::TrayMenuItemKind::Action,
            true
        },
        { model::TrayActionId::Unknown, {}, model::TrayMenuItemKind::Separator, false },
        {
            model::TrayActionId::Quit,
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::MenuQuit),
            model::TrayMenuItemKind::Action,
            true
        }
    };

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_menuItems = std::move(menuItems);
    }
    fireNotification(&ITrayViewModelCallback::onMenuChanged);
}

void TrayViewModel::handleMenuAction(model::TrayActionId actionId)
{
    bool available = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        available = std::any_of(m_menuItems.begin(), m_menuItems.end(),
            [actionId](const auto& item) {
                return item.actionId == actionId
                    && item.kind == model::TrayMenuItemKind::Action
                    && item.enabled;
            });
    }
    if (!available)
    {
        TRAY_VIEW_MODEL_LOG_WARN("handleMenuAction: unavailable action " << static_cast<int>(actionId));
        return;
    }

    fireNotification(&ITrayViewModelCallback::onActionRequested, actionId);
}

} // namespace commonHead::viewModels
