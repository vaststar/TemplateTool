#pragma once

#include <mutex>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/TrayViewModel/ITrayViewModel.h>

namespace commonHead::viewModels {

class TrayViewModel final
    : public ITrayViewModel
    , public commonHead::utilities::VMNotificationHelper<ITrayViewModelCallback>
{
public:
    explicit TrayViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    ~TrayViewModel() override;

    std::string getViewModelName() const override;
    std::vector<model::TrayMenuItemData> getMenuItems() const override;
    void reloadMenu() override;
    void handleMenuAction(model::TrayActionId actionId) override;

protected:
    void init() override;

private:
    mutable std::mutex m_mutex;
    std::vector<model::TrayMenuItemData> m_menuItems;
};

} // namespace commonHead::viewModels
