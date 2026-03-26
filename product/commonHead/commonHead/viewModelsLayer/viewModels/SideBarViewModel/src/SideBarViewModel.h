#pragma once

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/SideBarViewModel/ISideBarViewModel.h>

#include <mutex>
#include <optional>

namespace commonHead::viewModels {

class SideBarViewModel : public virtual ISideBarViewModel,
                         public virtual commonHead::utilities::VMNotificationHelper<ISideBarViewModelCallback>
{
public:
    explicit SideBarViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    ~SideBarViewModel() override;

public:
    // IViewModel
    std::string getViewModelName() const override;
    
    // ISideBarViewModel - Data access
    std::vector<model::NavItemData> getTopNavItems() const override;
    std::vector<model::NavItemData> getBottomNavItems() const override;
    model::PageId getCurrentPageId() const override;
    model::NavItemData getNavItem(model::PageId pageId) const override;
    
    // ISideBarViewModel - Operations
    bool navigateTo(model::PageId pageId, bool isUserAction = true) override;
    void updateBadge(model::PageId pageId, int32_t badge) override;
    void setNavItemState(model::PageId pageId, model::NavItemState state) override;
    void reloadNavConfig() override;

protected:
    void init() override;

private:
    void initDefaultNavItems();
    
    // Find nav item by pageId (thread-safe)
    // Returns nullopt if not found
    std::optional<model::NavItemData> findNavItem(model::PageId pageId) const;

private:
    mutable std::mutex m_mutex;
    std::vector<model::NavItemData> m_navItems;
    model::PageId m_currentPageId = model::PageId::Home;
};

} // namespace commonHead::viewModels
