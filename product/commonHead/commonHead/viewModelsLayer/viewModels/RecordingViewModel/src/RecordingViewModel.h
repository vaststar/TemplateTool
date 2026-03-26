#pragma once

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/RecordingViewModel/IRecordingViewModel.h>

#include <ucf/Agents/ScreenRecordingAgent/IScreenRecordingAgent.h>
#include <ucf/Agents/ScreenRecordingAgent/IScreenRecordingAgentCallback.h>
#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#include <memory>
#include <mutex>

namespace commonHead::viewModels {

/// Thin ViewModel that delegates recording lifecycle to ScreenRecordingAgent.
///
/// Responsibilities kept here:
///   - FFmpeg discovery (setAppDir / getFFmpegPath)
///   - Settings management (load/persist via FeatureSettingsService)
///   - Output-path generation
///   - GIF conversion (one-shot, no state)
///   - Translating agent callbacks → IRecordingViewModelCallback
class RecordingViewModel
    : public virtual IRecordingViewModel
    , public virtual commonHead::utilities::VMNotificationHelper<IRecordingViewModelCallback>
    , public virtual ucf::agents::IScreenRecordingAgentCallback
    , public std::enable_shared_from_this<RecordingViewModel>
{
public:
    explicit RecordingViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~RecordingViewModel() override;

    std::string getViewModelName() const override;

    // === Recording Control ===
    void startRecording(int displayIndex = 0) override;
    void startRegionRecording(int x, int y, int w, int h) override;
    void stopRecording() override;
    void abortRecording() override;
    void pauseRecording() override;
    void resumeRecording() override;
    void convertToGif(const std::string& inputPath, const std::string& outputPath) override;

    // === State Query ===
    model::RecordingState getState() const override;
    int getDuration() const override;
    bool isFFmpegAvailable() const override;
    std::string getFFmpegPath() const override;
    void setAppDir(const std::string& appDir) override;

    // === Settings ===
    model::RecordingSettings getSettings() const override;
    void updateSettings(const model::RecordingSettings& settings) override;

protected:
    void init() override;

private:
    // ── Agent callback translation (IScreenRecordingAgentCallback) ──
    void onAgentStateChanged(ucf::agents::RecordingAgentState state) override;
    void onRecordingStarted() override;
    void onRecordingPaused() override;
    void onRecordingResumed() override;
    void onDurationChanged(int seconds) override;
    void onRecordingCompleted(const std::string& outputPath) override;
    void onRecordingAborted() override;
    void onError(const std::string& message) override;

    // ── Helpers ──
    std::string generateOutputPath() const;

private:
    mutable std::mutex m_mutex;
    model::RecordingSettings m_settings;
    std::string m_ffmpegPath;
    std::string m_appDir;

    std::shared_ptr<ucf::agents::IScreenRecordingAgent> m_agent;
};

} // namespace commonHead::viewModels
