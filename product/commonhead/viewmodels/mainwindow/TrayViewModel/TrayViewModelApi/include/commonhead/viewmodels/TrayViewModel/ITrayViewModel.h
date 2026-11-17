#pragma once

#include <vector>

#include <commonhead/utilities/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>
#include <commonhead/viewmodels/TrayViewModel/TrayMenuModel.h>

namespace commonHead::viewModels {

class ITrayViewModelCallback
{
public:
    virtual ~ITrayViewModelCallback() = default;

    // Read the latest snapshot with getMenuItems(), including after late subscription.
    virtual void onMenuChanged() = 0;
    virtual void onActionRequested(model::TrayActionId actionId) = 0;
};

class ITrayViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<ITrayViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    ~ITrayViewModel() override = default;

    virtual std::vector<model::TrayMenuItemData> getMenuItems() const = 0;
    virtual void reloadMenu() = 0;
    virtual void handleMenuAction(model::TrayActionId actionId) = 0;
};

} // namespace commonHead::viewModels
