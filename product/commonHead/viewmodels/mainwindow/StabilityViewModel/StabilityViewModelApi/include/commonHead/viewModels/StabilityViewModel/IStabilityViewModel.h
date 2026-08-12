#pragma once

#include <string>

#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

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
