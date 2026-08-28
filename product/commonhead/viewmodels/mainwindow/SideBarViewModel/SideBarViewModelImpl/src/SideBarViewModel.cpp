#include "SideBarViewModel.h"
#include "LoggerDefine.h"

#include <algorithm>
#include <utility>

#include <commonhead/ResourceStringLoader/ResourceString.h>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ResourceLoader/IResourceLoader.h>
#include <commonhead/viewmodels/SideBarViewModel/SideBarViewModelCreator.h>

namespace commonHead::viewModels {

namespace impl {

std::shared_ptr<ISideBarViewModel> createSideBarViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<SideBarViewModel>(commonHeadFramework);
}

} // namespace impl

SideBarViewModel::SideBarViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : ISideBarViewModel(commonHeadFramework)
{
    SIDE_BAR_VIEW_MODEL_LOG_DEBUG("SideBarViewModel constructed, address: " << this);
}

SideBarViewModel::~SideBarViewModel()
{
    SIDE_BAR_VIEW_MODEL_LOG_DEBUG("SideBarViewModel destroying, address: " << this);
}

std::string SideBarViewModel::getViewModelName() const
{
    return "SideBarViewModel";
}

void SideBarViewModel::init()
{
    SIDE_BAR_VIEW_MODEL_LOG_DEBUG("SideBarViewModel::init");

    if (!initDefaultNavItems())
    {
        SIDE_BAR_VIEW_MODEL_LOG_ERROR("SideBarViewModel::init: failed to initialize navigation items");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_ready = true;
    }
    fireNotification(&ISideBarViewModelCallback::onSideBarReady);
}

std::shared_ptr<commonHead::IResourceLoader> SideBarViewModel::lockResourceLoader() const
{
    auto framework = getCommonHeadFramework().lock();
    if (!framework)
    {
        SIDE_BAR_VIEW_MODEL_LOG_ERROR(
            "SideBarViewModel::lockResourceLoader: CommonHeadFramework is not available");
        return {};
    }

    auto resourceLoader = framework->getResourceLoader();
    if (!resourceLoader)
    {
        SIDE_BAR_VIEW_MODEL_LOG_ERROR(
            "SideBarViewModel::lockResourceLoader: ResourceLoader is not available");
        return {};
    }

    return resourceLoader;
}

bool SideBarViewModel::initDefaultNavItems()
{
    auto resourceLoader = lockResourceLoader();
    if (!resourceLoader)
    {
        return false;
    }

    std::vector<model::NavItemData> navItems =
    {
        {
            model::PageId::Home,
            "home",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavHome),
            commonHead::model::AssetImageToken::Nav_Home,
            commonHead::model::AssetImageToken::Nav_Home_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Top,
            0,
            {}
        },
        {
            model::PageId::Contacts,
            "contacts",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavContacts),
            commonHead::model::AssetImageToken::Nav_Contacts,
            commonHead::model::AssetImageToken::Nav_Contacts_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Top,
            1,
            {}
        },
        {
            model::PageId::Tasks,
            "tasks",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavTasks),
            commonHead::model::AssetImageToken::Nav_Tasks,
            commonHead::model::AssetImageToken::Nav_Tasks_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Top,
            2,
            {}
        },
        {
            model::PageId::Credentials,
            "credentials",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavCredentials),
            commonHead::model::AssetImageToken::Nav_Credentials,
            commonHead::model::AssetImageToken::Nav_Credentials_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Top,
            3,
            {}
        },
        {
            model::PageId::Toolbox,
            "toolbox",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavToolbox),
            commonHead::model::AssetImageToken::Nav_Toolbox,
            commonHead::model::AssetImageToken::Nav_Toolbox_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Top,
            4,
            {}
        },
        {
            model::PageId::MiniApps,
            "miniApps",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavMiniApps),
            commonHead::model::AssetImageToken::Nav_Mini_Apps,
            commonHead::model::AssetImageToken::Nav_Mini_Apps_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Top,
            5,
            {}
        },
        {
            model::PageId::Settings,
            "settings",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavSettings),
            commonHead::model::AssetImageToken::Nav_Settings,
            commonHead::model::AssetImageToken::Nav_Settings_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Bottom,
            0,
            {}
        },
        {
            model::PageId::Help,
            "help",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavHelp),
            commonHead::model::AssetImageToken::Nav_Help,
            commonHead::model::AssetImageToken::Nav_Help_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Bottom,
            1,
            {
                { model::MenuActionId::CheckUpgrade,
                  resourceLoader->getLocalizedString(commonHead::model::LocalizedString::MenuCheckUpgrade) },
                { model::MenuActionId::About,
                  resourceLoader->getLocalizedString(commonHead::model::LocalizedString::MenuAbout) },
            }
        },
    };

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_navItems = std::move(navItems);
    }

    return true;
}

std::optional<model::NavItemData> SideBarViewModel::findNavItem(model::PageId pageId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& item : m_navItems)
    {
        if (item.pageId == pageId)
        {
            return item;
        }
    }
    return std::nullopt;
}

std::vector<model::NavItemData> SideBarViewModel::getTopNavItems() const
{
    std::vector<model::NavItemData> result;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& item : m_navItems)
        {
            if (item.isTopItem() && item.isVisible())
            {
                result.push_back(item);
            }
        }
    }

    std::sort(result.begin(), result.end(),
        [](const auto& a, const auto& b) { return a.sortOrder < b.sortOrder; });

    return result;
}

std::vector<model::NavItemData> SideBarViewModel::getBottomNavItems() const
{
    std::vector<model::NavItemData> result;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& item : m_navItems)
        {
            if (item.isBottomItem() && item.isVisible())
            {
                result.push_back(item);
            }
        }
    }

    std::sort(result.begin(), result.end(),
        [](const auto& a, const auto& b) { return a.sortOrder < b.sortOrder; });

    return result;
}

std::vector<model::NavItemData> SideBarViewModel::getNavItems() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_navItems;
}

bool SideBarViewModel::isSideBarReady() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_ready;
}

model::PageId SideBarViewModel::getDefaultPageId() const
{
    return model::PageId::Home;
}

model::NavItemData SideBarViewModel::getNavItem(model::PageId pageId) const
{
    auto item = findNavItem(pageId);
    return item.value_or(model::NavItemData{});
}

bool SideBarViewModel::navigateTo(model::PageId pageId, bool isUserAction)
{
    if (pageId == model::PageId::Unknown)
    {
        return false;
    }

    std::vector<model::SubMenuItem> subItems;
    bool exists = false;
    bool enabled = false;
    bool visible = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = std::find_if(m_navItems.begin(), m_navItems.end(),
            [pageId](const auto& item) { return item.pageId == pageId; });

        if (it != m_navItems.end())
        {
            exists = true;
            enabled = it->isEnabled();
            visible = it->isVisible();
            if (it->hasSubMenu())
            {
                subItems = it->subMenuItems;
            }
        }
    }

    if (!subItems.empty())
    {
        fireNotification(&ISideBarViewModelCallback::onSubMenuRequested, pageId, subItems);
        return false;
    }

    if (!exists)
    {
        SIDE_BAR_VIEW_MODEL_LOG_WARN("navigateTo: pageId " << static_cast<int>(pageId) << " not found");
        return false;
    }
    if (!enabled)
    {
        SIDE_BAR_VIEW_MODEL_LOG_WARN("navigateTo: pageId " << static_cast<int>(pageId) << " is disabled");
        return false;
    }
    if (!visible)
    {
        SIDE_BAR_VIEW_MODEL_LOG_WARN("navigateTo: pageId " << static_cast<int>(pageId) << " is hidden");
        return false;
    }

    SIDE_BAR_VIEW_MODEL_LOG_DEBUG("navigateTo: pageId=" << static_cast<int>(pageId)
        << " approved (userAction=" << isUserAction << ")");
    return true;
}

void SideBarViewModel::updateBadge(model::PageId pageId, std::int32_t badge)
{
    model::NavItemData updatedItem;
    bool found = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto& item : m_navItems)
        {
            if (item.pageId == pageId)
            {
                if (item.badge == badge)
                {
                    return;
                }
                item.badge = badge;
                updatedItem = item;
                found = true;
                break;
            }
        }

        if (!found)
        {
            SIDE_BAR_VIEW_MODEL_LOG_WARN("updateBadge: pageId " << static_cast<int>(pageId) << " not found");
            return;
        }
    }

    fireNotification(&ISideBarViewModelCallback::onNavItemsUpdated,
                     std::vector<model::NavItemData>{updatedItem});
    SIDE_BAR_VIEW_MODEL_LOG_DEBUG("Badge updated: pageId=" << static_cast<int>(pageId) << ", badge=" << badge);
}

void SideBarViewModel::setNavItemState(model::PageId pageId, model::NavItemState state)
{
    model::NavItemData updatedItem;
    bool found = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto& item : m_navItems)
        {
            if (item.pageId == pageId)
            {
                if (item.state == state)
                {
                    return;
                }
                item.state = state;
                updatedItem = item;
                found = true;
                break;
            }
        }

        if (!found)
        {
            SIDE_BAR_VIEW_MODEL_LOG_WARN("setNavItemState: pageId " << static_cast<int>(pageId) << " not found");
            return;
        }
    }

    fireNotification(&ISideBarViewModelCallback::onNavItemsUpdated,
                     std::vector<model::NavItemData>{updatedItem});
    SIDE_BAR_VIEW_MODEL_LOG_DEBUG("NavItem state updated: pageId=" << static_cast<int>(pageId)
        << ", state=" << static_cast<int>(state));
}

void SideBarViewModel::reloadNavConfig()
{
    SIDE_BAR_VIEW_MODEL_LOG_DEBUG("reloadNavConfig");

    std::vector<model::PageId> oldPageIds;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        oldPageIds.reserve(m_navItems.size());
        for (const auto& item : m_navItems)
        {
            oldPageIds.push_back(item.pageId);
        }
    }

    if (!initDefaultNavItems())
    {
        SIDE_BAR_VIEW_MODEL_LOG_ERROR("reloadNavConfig: failed to initialize navigation items");
        return;
    }

    std::vector<model::NavItemData> newItems;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        newItems = m_navItems;
    }

    std::vector<model::NavItemData> added;
    std::vector<model::NavItemData> updated;
    std::vector<model::PageId> removed;

    for (const auto& item : newItems)
    {
        const bool wasPresent = std::find(oldPageIds.begin(), oldPageIds.end(), item.pageId)
                                != oldPageIds.end();
        if (wasPresent)
        {
            updated.push_back(item);
        }
        else
        {
            added.push_back(item);
        }
    }
    for (const auto& oldId : oldPageIds)
    {
        const bool stillPresent = std::find_if(newItems.begin(), newItems.end(),
            [oldId](const auto& item) { return item.pageId == oldId; }) != newItems.end();
        if (!stillPresent)
        {
            removed.push_back(oldId);
        }
    }

    if (!removed.empty())
    {
        fireNotification(&ISideBarViewModelCallback::onNavItemsRemoved, removed);
    }
    if (!added.empty())
    {
        fireNotification(&ISideBarViewModelCallback::onNavItemsAdded, added);
    }
    if (!updated.empty())
    {
        fireNotification(&ISideBarViewModelCallback::onNavItemsUpdated, updated);
    }
}

void SideBarViewModel::handleSubMenuAction(model::MenuActionId actionId)
{
    SIDE_BAR_VIEW_MODEL_LOG_DEBUG("handleSubMenuAction: " << static_cast<int>(actionId));

    if (actionId == model::MenuActionId::Unknown)
    {
        SIDE_BAR_VIEW_MODEL_LOG_WARN("Unknown MenuActionId: " << static_cast<int>(actionId));
        return;
    }

    fireNotification(&ISideBarViewModelCallback::onMenuActionClicked, actionId);
}

} // namespace commonHead::viewModels
