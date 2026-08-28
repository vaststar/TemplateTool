#pragma once

#include <memory>
#include <mutex>
#include <optional>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/SideBarViewModel/ISideBarViewModel.h>

namespace commonHead {
class IResourceLoader;
}

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
    std::vector<model::NavItemData> getNavItems() const override;
    [[nodiscard]] bool isSideBarReady() const override;
    model::PageId getDefaultPageId() const override;
    model::NavItemData getNavItem(model::PageId pageId) const override;

    // ISideBarViewModel - Operations
    bool navigateTo(model::PageId pageId, bool isUserAction = true) override;
    void updateBadge(model::PageId pageId, std::int32_t badge) override;
    void setNavItemState(model::PageId pageId, model::NavItemState state) override;
    void reloadNavConfig() override;
    void handleSubMenuAction(model::MenuActionId actionId) override;

protected:
    void init() override;

private:
    [[nodiscard]] std::shared_ptr<commonHead::IResourceLoader> lockResourceLoader() const;
    [[nodiscard]] bool initDefaultNavItems();

    // Find nav item by pageId (thread-safe)
    // Returns nullopt if not found
    std::optional<model::NavItemData> findNavItem(model::PageId pageId) const;

private:
    mutable std::mutex m_mutex;
    std::vector<model::NavItemData> m_navItems;
    bool m_ready = false;
};

} // namespace commonHead::viewModels
