#pragma once

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/RecordingViewModel/IRecordingViewModel.h>

#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace commonHead::viewModels {

class RecordingViewModel
    : public virtual IRecordingViewModel
    , public virtual commonHead::utilities::VMNotificationHelper<IRecordingViewModelCallback>
{
public:
    explicit RecordingViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~RecordingViewModel() override;

    std::string getViewModelName() const override;

    // === Recording Control ===
    void startRecording(int displayIndex = 0) override;
    void startRegionRecording(int x, int y, int w, int h) override;
    void stopRecording() override;
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
    void setState(model::RecordingState newState);
    std::string generateOutputPath() const;
    void startDurationTimer();
    void stopDurationTimer();
    void doStartRecording(const ucf::utilities::screenrecording::RecordingConfig& config);

private:
    mutable std::mutex m_mutex;
    model::RecordingSettings m_settings;
    model::RecordingState m_state = model::RecordingState::Idle;
    std::string m_ffmpegPath;
    std::string m_appDir;

    // Active recording session
    ucf::utilities::screenrecording::RecordingSession m_session;

    // Duration timer
    std::atomic<int> m_duration{0};
    std::atomic<bool> m_timerRunning{false};
    std::atomic<bool> m_timerShouldExit{false};
    std::thread m_timerThread;
    std::condition_variable m_timerCv;
    std::mutex m_timerMutex;

    // Async stop
    std::atomic<bool> m_stopping{false};
    std::thread m_stopThread;
};

} // namespace commonHead::viewModels
