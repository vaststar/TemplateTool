#pragma once

#include <AssetToken.h>

#include <cstdint>
#include <string>
#include <vector>

#include <commonHead/viewModels/SideBarViewModel/SideBarViewModelTypesExport.h>

namespace commonHead::viewModels::model {

/**
 * @brief Page ID enumeration
 */
enum class PageId : std::uint8_t
{
    Unknown = 0,
    Home,           // Home page
    Contacts,       // Contacts
    Tasks,          // Work plans
    Credentials,    // My credentials
    Toolbox,        // Toolbox
    MiniApps,       // Mini apps
    Settings,       // Settings
    Help,           // Help
};

/**
 * @brief Navigation item state
 */
enum class NavItemState : std::uint8_t
{
    Normal = 0,
    Disabled,
    Hidden
};

/**
 * @brief Navigation item position
 */
enum class NavItemPosition : std::uint8_t
{
    Top = 0,        // Top area (main navigation)
    Bottom          // Bottom area (settings, help, etc.)
};

/**
 * @brief Menu action ID for submenu items
 */
enum class MenuActionId : std::uint8_t
{
    Unknown = 0,
    CheckUpgrade,
    About
};

/**
 * @brief Submenu item data
 */
struct SIDE_BAR_VIEW_MODEL_TYPES_API SubMenuItem
{
    MenuActionId actionId = MenuActionId::Unknown;
    std::string title;
    commonHead::model::AssetImageToken icon = commonHead::model::AssetImageToken::None;
};

/**
 * @brief Navigation item data structure
 */
struct SIDE_BAR_VIEW_MODEL_TYPES_API NavItemData
{
    PageId pageId = PageId::Unknown;
    std::string id;                     // Unique identifier
    std::string title;                  // Display title
    commonHead::model::AssetImageToken icon = commonHead::model::AssetImageToken::None;           // Normal state icon token
    commonHead::model::AssetImageToken iconSelected = commonHead::model::AssetImageToken::None;   // Selected state icon token
    std::int32_t badge = 0;             // Badge number (0 means hidden)
    NavItemState state = NavItemState::Normal;
    NavItemPosition position = NavItemPosition::Top;
    std::int32_t sortOrder = 0;         // Sort weight (within same position)
    std::vector<SubMenuItem> subMenuItems;  // Popup submenu items (empty = normal nav)

    bool operator==(const NavItemData& other) const
    {
        return pageId == other.pageId;
    }

    bool isEnabled() const
    {
        return state == NavItemState::Normal;
    }

    bool isVisible() const
    {
        return state != NavItemState::Hidden;
    }

    bool isTopItem() const
    {
        return position == NavItemPosition::Top;
    }

    bool isBottomItem() const
    {
        return position == NavItemPosition::Bottom;
    }

    bool hasSubMenu() const
    {
        return !subMenuItems.empty();
    }
};

} // namespace commonHead::viewModels::model
