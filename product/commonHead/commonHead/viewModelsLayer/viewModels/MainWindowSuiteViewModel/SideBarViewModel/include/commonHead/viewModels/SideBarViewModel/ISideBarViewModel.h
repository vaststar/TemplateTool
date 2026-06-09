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
    // Fired once after the initial nav config is built. Late subscribers
    // should consult isSideBarReady() and seed from getNavItems().
    virtual void onSideBarReady() {}

    virtual void onNavItemsAdded(const std::vector<model::NavItemData>& /*items*/) {}
    virtual void onNavItemsUpdated(const std::vector<model::NavItemData>& /*items*/) {}
    virtual void onNavItemsRemoved(const std::vector<model::PageId>& /*pageIds*/) {}

    // Nav item with submenu was clicked — UI should show the popup.
    virtual void onSubMenuRequested(model::PageId pageId,
                                    const std::vector<model::SubMenuItem>& items) = 0;

    // Submenu action needs UI-layer handling.
    virtual void onMenuActionClicked(model::MenuActionId actionId) = 0;
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

    virtual std::vector<model::NavItemData> getTopNavItems() const = 0;
    virtual std::vector<model::NavItemData> getBottomNavItems() const = 0;

    // Full snapshot for late subscribers (unsorted, top + bottom).
    virtual std::vector<model::NavItemData> getNavItems() const = 0;

    [[nodiscard]] virtual bool isSideBarReady() const = 0;

    // Default page id for controller seeding. VM owns no UI selection state.
    virtual model::PageId getDefaultPageId() const = 0;

    virtual model::NavItemData getNavItem(model::PageId pageId) const = 0;

    // ========== Operations ==========

    // Validation gate — does NOT mutate any selection state (controller owns it).
    // Returns true if the controller should switch to pageId. Submenu items fire
    // onSubMenuRequested and return false.
    virtual bool navigateTo(model::PageId pageId, bool isUserAction = true) = 0;

    virtual void updateBadge(model::PageId pageId, int32_t badge) = 0;
    virtual void setNavItemState(model::PageId pageId, model::NavItemState state) = 0;
    virtual void reloadNavConfig() = 0;

    /**
     * @brief Handle a submenu action chosen by the user
     * @param actionId The action that was triggered
     */
    virtual void handleSubMenuAction(model::MenuActionId actionId) = 0;

public:
    static std::shared_ptr<ISideBarViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
