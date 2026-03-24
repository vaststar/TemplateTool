#pragma once

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/RecordingViewModel/IRecordingViewModel.h>

#include <mutex>

namespace commonHead::viewModels {

class RecordingViewModel
    : public virtual IRecordingViewModel
    , public virtual commonHead::utilities::VMNotificationHelper<IRecordingViewModelCallback>
{
public:
    explicit RecordingViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~RecordingViewModel() override = default;

    std::string getViewModelName() const override;

    // === Settings ===
    model::RecordingSettings getSettings() const override;
    void updateSettings(const model::RecordingSettings& settings) override;

protected:
    void init() override;

private:
    mutable std::mutex m_mutex;
    model::RecordingSettings m_settings;
};

} // namespace commonHead::viewModels
