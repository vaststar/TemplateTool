#include "SideBarViewModel.h"

#include <algorithm>
#include <AssetToken.h>
#include <ResourceString.h>
#include <magic_enum/magic_enum.hpp>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

namespace commonHead::viewModels {

// ==================== Factory method ====================

std::shared_ptr<ISideBarViewModel> ISideBarViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<SideBarViewModel>(commonHeadFramework);
}

// ==================== Constructor/Destructor ====================

SideBarViewModel::SideBarViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : ISideBarViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create SideBarViewModel");
}

SideBarViewModel::~SideBarViewModel()
{
    COMMONHEAD_LOG_DEBUG("destroy SideBarViewModel");
}

// ==================== IViewModel ====================

std::string SideBarViewModel::getViewModelName() const
{
    return "SideBarViewModel";
}

void SideBarViewModel::init()
{
    COMMONHEAD_LOG_DEBUG("SideBarViewModel::init");
    initDefaultNavItems();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_ready = true;
    }
    fireNotification(&ISideBarViewModelCallback::onSideBarReady);
}

// ==================== Initialization ====================

void SideBarViewModel::initDefaultNavItems()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto resourceLoader = getCommonHeadFramework().lock()->getResourceLoader();
    m_navItems =
    {
        // ========== Top navigation items ==========
        {
            model::PageId::Home,
            "home",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavHome),
            commonHead::model::AssetImageToken::Nav_Home,
            commonHead::model::AssetImageToken::Nav_Home_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Top,
            0
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
            1
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
            2
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
            3
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
            4
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
            5
        },

        // ========== Bottom navigation items ==========
        {
            model::PageId::Settings,
            "settings",
            resourceLoader->getLocalizedString(commonHead::model::LocalizedString::NavSettings),
            commonHead::model::AssetImageToken::Nav_Settings,
            commonHead::model::AssetImageToken::Nav_Settings_Selected,
            0,
            model::NavItemState::Normal,
            model::NavItemPosition::Bottom,
            0
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
            // Submenu items
            {
                { model::MenuActionId::CheckUpgrade,
                  resourceLoader->getLocalizedString(commonHead::model::LocalizedString::MenuCheckUpgrade) },
                { model::MenuActionId::About,
                  resourceLoader->getLocalizedString(commonHead::model::LocalizedString::MenuAbout) },
            }
        },
    };
}

// ==================== Helper method ====================

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

// ==================== Data access ====================

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

    // Sort outside of lock, result is a local copy
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

    // Sort outside of lock, result is a local copy
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

// ==================== Operations ====================

bool SideBarViewModel::navigateTo(model::PageId pageId, bool isUserAction)
{
    if (pageId == model::PageId::Unknown)
    {
        return false;
    }

    // Submenu items are intercepted and turned into onSubMenuRequested (return false).
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
        COMMONHEAD_LOG_WARN("navigateTo: pageId " << magic_enum::enum_name(pageId) << " not found");
        return false;
    }
    if (!enabled)
    {
        COMMONHEAD_LOG_WARN("navigateTo: pageId " << magic_enum::enum_name(pageId) << " is disabled");
        return false;
    }
    if (!visible)
    {
        COMMONHEAD_LOG_WARN("navigateTo: pageId " << magic_enum::enum_name(pageId) << " is hidden");
        return false;
    }

    COMMONHEAD_LOG_DEBUG("navigateTo: pageId=" << magic_enum::enum_name(pageId)
        << " approved (userAction=" << isUserAction << ")");
    return true;
}

void SideBarViewModel::updateBadge(model::PageId pageId, int32_t badge)
{
    model::NavItemData updatedItem;
    bool found = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Find and update directly in the loop
        for (auto& item : m_navItems)
        {
            if (item.pageId == pageId)
            {
                if (item.badge == badge)
                {
                    return;  // Value unchanged
                }
                item.badge = badge;
                updatedItem = item;
                found = true;
                break;
            }
        }

        if (!found)
        {
            COMMONHEAD_LOG_WARN("updateBadge: pageId " << magic_enum::enum_name(pageId) << " not found");
            return;
        }
    }

    // Fire notification outside of lock to avoid deadlock
    fireNotification(&ISideBarViewModelCallback::onNavItemsUpdated,
                     std::vector<model::NavItemData>{updatedItem});
    COMMONHEAD_LOG_DEBUG("Badge updated: pageId=" << magic_enum::enum_name(pageId) << ", badge=" << badge);
}

void SideBarViewModel::setNavItemState(model::PageId pageId, model::NavItemState state)
{
    model::NavItemData updatedItem;
    bool found = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Find and update directly in the loop
        for (auto& item : m_navItems)
        {
            if (item.pageId == pageId)
            {
                if (item.state == state)
                {
                    return;  // State unchanged
                }
                item.state = state;
                updatedItem = item;
                found = true;
                break;
            }
        }

        if (!found)
        {
            COMMONHEAD_LOG_WARN("setNavItemState: pageId " << magic_enum::enum_name(pageId) << " not found");
            return;
        }
    }

    // Fire notification outside of lock to avoid deadlock
    fireNotification(&ISideBarViewModelCallback::onNavItemsUpdated,
                     std::vector<model::NavItemData>{updatedItem});
    COMMONHEAD_LOG_DEBUG("NavItem state updated: pageId=" << magic_enum::enum_name(pageId)
        << ", state=" << magic_enum::enum_name(state));
}

void SideBarViewModel::reloadNavConfig()
{
    COMMONHEAD_LOG_DEBUG("reloadNavConfig");

    // Diff old vs new to fire fine-grained added/updated/removed.
    std::vector<model::PageId> oldPageIds;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        oldPageIds.reserve(m_navItems.size());
        for (const auto& item : m_navItems)
        {
            oldPageIds.push_back(item.pageId);
        }
    }

    initDefaultNavItems();

    std::vector<model::NavItemData> newItems;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        newItems = m_navItems;
    }

    std::vector<model::NavItemData> added;
    std::vector<model::NavItemData> updated;
    std::vector<model::PageId>      removed;

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

    // Fire notifications outside of lock to avoid deadlock
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
    COMMONHEAD_LOG_DEBUG("handleSubMenuAction: " << magic_enum::enum_name(actionId));

    if (actionId == model::MenuActionId::Unknown)
    {
        COMMONHEAD_LOG_WARN("Unknown MenuActionId: " << magic_enum::enum_name(actionId));
        return;
    }

    fireNotification(&ISideBarViewModelCallback::onMenuActionClicked, actionId);
}

} // namespace commonHead::viewModels
