#pragma once

#include <memory>
#include <string>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/StabilityViewModel/IStabilityViewModel.h>

namespace commonHead::viewModels{

class StabilityViewModel: public virtual IStabilityViewModel,
                          public virtual commonHead::utilities::VMNotificationHelper<IStabilityViewModelCallback>,
                          public std::enable_shared_from_this<StabilityViewModel>
{
public:
    explicit StabilityViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    StabilityViewModel(const StabilityViewModel&) = delete;
    StabilityViewModel(StabilityViewModel&&) = delete;
    StabilityViewModel& operator=(const StabilityViewModel&) = delete;
    StabilityViewModel& operator=(StabilityViewModel&&) = delete;
    ~StabilityViewModel() override;

public:
    std::string getViewModelName() const override;
    void reportHeartbeat() override;
    int getHeartbeatIntervalMs() const override;

protected:
    void init() override;

private:
    static constexpr int DEFAULT_HEARTBEAT_INTERVAL_MS = 1000;
};

} // namespace commonHead::viewModels
