#pragma once

#include <memory>
#include <string>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/StabilityViewModel/IStabilityViewModel.h>

namespace commonHead::viewModels{

class StabilityViewModel: public virtual IStabilityViewModel, 
                          public virtual commonHead::utilities::VMNotificationHelper<IStabilityViewModelCallback>,
                          public std::enable_shared_from_this<StabilityViewModel>
{
public:
    explicit StabilityViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    StabilityViewModel() = default;
    StabilityViewModel(const StabilityViewModel&) = delete;
    StabilityViewModel(StabilityViewModel&&) = delete;
    StabilityViewModel& operator=(const StabilityViewModel&) = delete;
    StabilityViewModel& operator=(StabilityViewModel&&) = delete;
    ~StabilityViewModel();

public:
    virtual std::string getViewModelName() const override;
    virtual void reportHeartbeat() override;
    virtual int getHeartbeatIntervalMs() const override;

protected:
    virtual void init() override;

private:
    static constexpr int DEFAULT_HEARTBEAT_INTERVAL_MS = 1000;
};

} // namespace commonHead::viewModels
