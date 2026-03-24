#pragma once

#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/RecordingViewModel/IRecordingModel.h>

namespace commonHead {
class ICommonHeadFramework;
using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
} // namespace commonHead

namespace commonHead::viewModels {

// ============================================================================
// Callback interface — UI layer implements this to receive notifications
// ============================================================================

class COMMONHEAD_EXPORT IRecordingViewModelCallback {
public:
    IRecordingViewModelCallback() = default;
    IRecordingViewModelCallback(const IRecordingViewModelCallback&) = delete;
    IRecordingViewModelCallback(IRecordingViewModelCallback&&) = delete;
    IRecordingViewModelCallback& operator=(const IRecordingViewModelCallback&) = delete;
    IRecordingViewModelCallback& operator=(IRecordingViewModelCallback&&) = delete;
    virtual ~IRecordingViewModelCallback() = default;

    /// Settings updated (e.g. from another source or DB load)
    virtual void onSettingsChanged(const model::RecordingSettings& settings) = 0;

    /// Error occurred
    virtual void onError(const std::string& message) = 0;
};

// ============================================================================
// IRecordingViewModel — pure interface
// ============================================================================

class COMMONHEAD_EXPORT IRecordingViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<IRecordingViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    ~IRecordingViewModel() override = default;

    // === Settings ===
    [[nodiscard]] virtual model::RecordingSettings getSettings() const = 0;
    virtual void updateSettings(const model::RecordingSettings& settings) = 0;

    // === Factory ===
    static std::shared_ptr<IRecordingViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
