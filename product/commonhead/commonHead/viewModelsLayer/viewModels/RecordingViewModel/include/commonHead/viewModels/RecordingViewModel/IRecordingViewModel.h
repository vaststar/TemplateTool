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

    /// Recording state machine transition (Idle / Recording / Paused)
    virtual void onStateChanged(model::RecordingState state) = 0;

    /// Duration tick (seconds elapsed)
    virtual void onDurationChanged(int seconds) = 0;

    /// Recording completed — output file path
    virtual void onRecordingCompleted(const std::string& filePath) = 0;

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

    // === Recording Control ===
    virtual void startRecording(int displayIndex = 0) = 0;
    virtual void startRegionRecording(int x, int y, int w, int h) = 0;
    virtual void stopRecording() = 0;
    virtual void abortRecording() = 0;
    virtual void pauseRecording() = 0;
    virtual void resumeRecording() = 0;
    virtual void convertToGif(const std::string& inputPath, const std::string& outputPath) = 0;

    // === State Query ===
    [[nodiscard]] virtual model::RecordingState getState() const = 0;
    [[nodiscard]] virtual int getDuration() const = 0;
    [[nodiscard]] virtual bool isFFmpegAvailable() const = 0;
    [[nodiscard]] virtual std::string getFFmpegPath() const = 0;

    /// Set the application binary directory for FFmpeg discovery.
    /// Must be called after initViewModel() and before startRecording().
    virtual void setAppDir(const std::string& appDir) = 0;

    // === Settings ===
    [[nodiscard]] virtual model::RecordingSettings getSettings() const = 0;
    virtual void updateSettings(const model::RecordingSettings& settings) = 0;

    // === Factory ===
    static std::shared_ptr<IRecordingViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
