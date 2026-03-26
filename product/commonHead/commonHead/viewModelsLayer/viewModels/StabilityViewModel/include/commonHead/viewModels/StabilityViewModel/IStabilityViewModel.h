#pragma once

#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{

class COMMONHEAD_EXPORT IStabilityViewModelCallback
{
public:
    IStabilityViewModelCallback() = default;
    IStabilityViewModelCallback(const IStabilityViewModelCallback&) = delete;
    IStabilityViewModelCallback(IStabilityViewModelCallback&&) = delete;
    IStabilityViewModelCallback& operator=(const IStabilityViewModelCallback&) = delete;
    IStabilityViewModelCallback& operator=(IStabilityViewModelCallback&&) = delete;
    virtual ~IStabilityViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IStabilityViewModel: public IViewModel, 
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
    virtual std::string getViewModelName() const = 0;
    
    /// @brief Report a heartbeat to indicate UI is responsive
    virtual void reportHeartbeat() = 0;
    
    /// @brief Get the recommended heartbeat interval in milliseconds
    [[nodiscard]] virtual int getHeartbeatIntervalMs() const = 0;
public:
    static std::shared_ptr<IStabilityViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
