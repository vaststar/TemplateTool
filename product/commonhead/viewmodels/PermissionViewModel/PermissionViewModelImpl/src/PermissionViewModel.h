#pragma once

#include <memory>
#include <mutex>
#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/PermissionViewModel/IPermissionViewModel.h>

namespace commonHead::viewModels {

class PermissionViewModel final
    : public IPermissionViewModel
    , public commonHead::utilities::VMNotificationHelper<IPermissionViewModelCallback>
    , public std::enable_shared_from_this<PermissionViewModel>
{
public:
    PermissionViewModel(commonHead::ICommonHeadFrameworkWptr framework, model::PermissionKind kind);
    ~PermissionViewModel() override = default;

    std::string getViewModelName() const override;
    model::PermissionState getState() const override;
    void refresh() override;
    void performAction() override;

protected:
    void init() override;

private:
    void finishAction(model::PermissionError error,
                      model::PermissionAction action = model::PermissionAction::None);
    void publishState() noexcept;

    const model::PermissionKind mKind;
    mutable std::mutex mMutex;
    model::PermissionState mState;
    bool mRequestAttempted = false;
};

} // namespace commonHead::viewModels
