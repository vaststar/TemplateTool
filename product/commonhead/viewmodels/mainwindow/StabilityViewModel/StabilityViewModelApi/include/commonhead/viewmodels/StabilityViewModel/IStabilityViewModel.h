#pragma once

#include <string>

#include <commonhead/utilities/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>

namespace commonHead::viewModels{

class IStabilityViewModelCallback
{
public:
    IStabilityViewModelCallback() = default;
    IStabilityViewModelCallback(const IStabilityViewModelCallback&) = delete;
    IStabilityViewModelCallback(IStabilityViewModelCallback&&) = delete;
    IStabilityViewModelCallback& operator=(const IStabilityViewModelCallback&) = delete;
    IStabilityViewModelCallback& operator=(IStabilityViewModelCallback&&) = delete;
    virtual ~IStabilityViewModelCallback() = default;
};

class IStabilityViewModel: public IViewModel,
    public virtual commonHead::utilities::IVMNotificationHelper<IStabilityViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IStabilityViewModel(const IStabilityViewModel&) = delete;
    IStabilityViewModel(IStabilityViewModel&&) = delete;
    IStabilityViewModel& operator=(const IStabilityViewModel&) = delete;
    IStabilityViewModel& operator=(IStabilityViewModel&&) = delete;
    virtual ~IStabilityViewModel() = default;
public:
    /// @brief Report a heartbeat to indicate UI is responsive
    virtual void reportHeartbeat() = 0;
    
    /// @brief Get the recommended heartbeat interval in milliseconds
    [[nodiscard]] virtual int getHeartbeatIntervalMs() const = 0;
};

} // namespace commonHead::viewModels
