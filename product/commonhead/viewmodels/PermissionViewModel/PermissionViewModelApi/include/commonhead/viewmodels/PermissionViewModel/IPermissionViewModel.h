#pragma once

#include <commonhead/utilities/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>
#include <commonhead/viewmodels/PermissionViewModel/PermissionModel.h>

namespace commonHead::viewModels {

class IPermissionViewModelCallback
{
public:
    virtual ~IPermissionViewModelCallback() = default;
    virtual void onPermissionStateChanged(const model::PermissionState& state) = 0;
};

class IPermissionViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<IPermissionViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    ~IPermissionViewModel() override = default;

    virtual model::PermissionState getState() const = 0;
    /// Requery without prompting. A native action in progress refreshes on completion.
    virtual void refresh() = 0;
    /// Perform the current Request/OpenSettings action; ignore repeated clicks while busy.
    virtual void performAction() = 0;
};

} // namespace commonHead::viewModels
