#pragma once

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <string>
#include <vector>
#include <cstdint>

namespace commonHead::viewModels::model {

/**
 * @brief Page ID enumeration
 */
enum class COMMONHEAD_EXPORT PageId : uint8_t {
    Unknown = 0,
    Home,           // Home page
    Contacts,       // Contacts
    Tasks,          // Work plans
    Credentials,    // My credentials
    Toolbox,        // Toolbox
    Settings,       // Settings
    Help,           // Help
    About           // About
};

/**
 * @brief Navigation item state
 */
enum class COMMONHEAD_EXPORT NavItemState : uint8_t {
    Normal = 0,
    Disabled,
    Hidden
};

/**
 * @brief Navigation item position
 */
enum class COMMONHEAD_EXPORT NavItemPosition : uint8_t {
    Top = 0,        // Top area (main navigation)
    Bottom          // Bottom area (settings, help, etc.)
};

/**
 * @brief Navigation item data structure
 */
struct COMMONHEAD_EXPORT NavItemData {
    PageId pageId = PageId::Unknown;
    std::string id;                     // Unique identifier
    std::string title;                  // Display title
    std::string icon;                   // Normal state icon resource path
    std::string iconSelected;           // Selected state icon resource path
    int32_t badge = 0;                  // Badge number (0 means hidden)
    NavItemState state = NavItemState::Normal;
    NavItemPosition position = NavItemPosition::Top;
    int32_t sortOrder = 0;              // Sort weight (within same position)
    
    bool operator==(const NavItemData& other) const {
        return pageId == other.pageId;
    }
    
    bool isEnabled() const {
        return state == NavItemState::Normal;
    }
    
    bool isVisible() const {
        return state != NavItemState::Hidden;
    }
    
    bool isTopItem() const {
        return position == NavItemPosition::Top;
    }
    
    bool isBottomItem() const {
        return position == NavItemPosition::Bottom;
    }
};

/**
 * @brief Page change event data
 */
struct COMMONHEAD_EXPORT PageChangeEvent {
    PageId fromPageId = PageId::Unknown;
    PageId toPageId = PageId::Unknown;
    bool isUserAction = false;
};

} // namespace commonHead::viewModels::model
