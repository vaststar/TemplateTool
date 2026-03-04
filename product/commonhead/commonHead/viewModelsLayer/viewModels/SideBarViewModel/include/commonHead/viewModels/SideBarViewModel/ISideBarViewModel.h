#pragma once

#include <vector>
#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/SideBarViewModel/SideBarModel.h>

namespace commonHead {
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels {

/**
 * @brief SideBar ViewModel callback interface
 */
class COMMONHEAD_EXPORT ISideBarViewModelCallback
{
public:
    ISideBarViewModelCallback() = default;
    ISideBarViewModelCallback(const ISideBarViewModelCallback&) = delete;
    ISideBarViewModelCallback(ISideBarViewModelCallback&&) = delete;
    ISideBarViewModelCallback& operator=(const ISideBarViewModelCallback&) = delete;
    ISideBarViewModelCallback& operator=(ISideBarViewModelCallback&&) = delete;
    virtual ~ISideBarViewModelCallback() = default;

public:
    /**
     * @brief Called when navigation items list changed (full list)
     * @param items All navigation items
     */
    virtual void onNavItemsChanged(const std::vector<model::NavItemData>& items) = 0;
    
    /**
     * @brief Called when current page changed
     * @param event Page change event
     */
    virtual void onCurrentPageChanged(const model::PageChangeEvent& event) = 0;
    
    /**
     * @brief Called when single navigation item updated (badge or state changed)
     * @param item Updated navigation item data
     */
    virtual void onNavItemUpdated(const model::NavItemData& item) = 0;
};

/**
 * @brief SideBar ViewModel interface
 */
class COMMONHEAD_EXPORT ISideBarViewModel 
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<ISideBarViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    ISideBarViewModel(const ISideBarViewModel&) = delete;
    ISideBarViewModel(ISideBarViewModel&&) = delete;
    ISideBarViewModel& operator=(const ISideBarViewModel&) = delete;
    ISideBarViewModel& operator=(ISideBarViewModel&&) = delete;
    virtual ~ISideBarViewModel() = default;

public:
    // ========== Data Access ==========
    
    virtual std::string getViewModelName() const override = 0;
    
    /**
     * @brief Get top navigation items (sorted)
     */
    virtual std::vector<model::NavItemData> getTopNavItems() const = 0;
    
    /**
     * @brief Get bottom navigation items (sorted)
     */
    virtual std::vector<model::NavItemData> getBottomNavItems() const = 0;
    
    /**
     * @brief Get current selected page ID
     */
    virtual model::PageId getCurrentPageId() const = 0;
    
    /**
     * @brief Get navigation item data for specified page
     * @param pageId Page ID
     * @return Navigation item data, returns Unknown if not exists
     */
    virtual model::NavItemData getNavItem(model::PageId pageId) const = 0;
    
    // ========== Operations ==========
    
    /**
     * @brief Navigate to specified page
     * @param pageId Target page ID
     * @param isUserAction Whether this is a user-initiated action
     * @return Whether navigation succeeded
     */
    virtual bool navigateTo(model::PageId pageId, bool isUserAction = true) = 0;
    
    /**
     * @brief Update badge for specified page
     * @param pageId Page ID
     * @param badge Badge value (0 to clear)
     */
    virtual void updateBadge(model::PageId pageId, int32_t badge) = 0;
    
    /**
     * @brief Set navigation item state
     * @param pageId Page ID
     * @param state New state
     */
    virtual void setNavItemState(model::PageId pageId, model::NavItemState state) = 0;
    
    /**
     * @brief Reload navigation configuration
     */
    virtual void reloadNavConfig() = 0;

public:
    static std::shared_ptr<ISideBarViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
