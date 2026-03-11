#include "SideBarViewModel.h"

#include <algorithm>
#include <AssetToken.h>
#include <ResourceString.h>

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
            1
        },
    };
    
    m_currentPageId = model::PageId::Home;
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

model::PageId SideBarViewModel::getCurrentPageId() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentPageId;
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
    
    model::PageChangeEvent event;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        const auto it = std::find_if(m_navItems.begin(), m_navItems.end(),
            [pageId](const auto& item) { return item.pageId == pageId; });
        
        if (it == m_navItems.end())
        {
            COMMONHEAD_LOG_WARN("navigateTo: pageId " << static_cast<int>(pageId) << " not found");
            return false;
        }
        
        if (!it->isEnabled())
        {
            COMMONHEAD_LOG_WARN("navigateTo: pageId " << static_cast<int>(pageId) << " is disabled");
            return false;
        }
        
        if (!it->isVisible())
        {
            COMMONHEAD_LOG_WARN("navigateTo: pageId " << static_cast<int>(pageId) << " is hidden");
            return false;
        }
        
        if (m_currentPageId == pageId)
        {
            return true;
        }
        
        event.fromPageId = m_currentPageId;
        event.toPageId = pageId;
        event.isUserAction = isUserAction;
        m_currentPageId = pageId;
    }
    
    // Fire notification outside of lock to avoid deadlock
    fireNotification(&ISideBarViewModelCallback::onCurrentPageChanged, event);
    
    COMMONHEAD_LOG_DEBUG("Navigate: " << static_cast<int>(event.fromPageId) 
        << " -> " << static_cast<int>(event.toPageId) 
        << " (userAction=" << isUserAction << ")");
    
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
            COMMONHEAD_LOG_WARN("updateBadge: pageId " << static_cast<int>(pageId) << " not found");
            return;
        }
    }
    
    // Fire notification outside of lock to avoid deadlock
    fireNotification(&ISideBarViewModelCallback::onNavItemUpdated, updatedItem);
    COMMONHEAD_LOG_DEBUG("Badge updated: pageId=" << static_cast<int>(pageId) << ", badge=" << badge);
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
            COMMONHEAD_LOG_WARN("setNavItemState: pageId " << static_cast<int>(pageId) << " not found");
            return;
        }
    }
    
    // Fire notification outside of lock to avoid deadlock
    fireNotification(&ISideBarViewModelCallback::onNavItemUpdated, updatedItem);
    COMMONHEAD_LOG_DEBUG("NavItem state updated: pageId=" << static_cast<int>(pageId) 
        << ", state=" << static_cast<int>(state));
}

void SideBarViewModel::reloadNavConfig()
{
    COMMONHEAD_LOG_DEBUG("reloadNavConfig");
    
    initDefaultNavItems();
    
    std::vector<model::NavItemData> items;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        items = m_navItems;
    }
    
    // Fire notification outside of lock to avoid deadlock
    fireNotification(&ISideBarViewModelCallback::onNavItemsChanged, items);
}

} // namespace commonHead::viewModels
