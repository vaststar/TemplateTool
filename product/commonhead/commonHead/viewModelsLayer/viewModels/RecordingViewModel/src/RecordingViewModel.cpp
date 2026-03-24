#include "RecordingViewModel.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <ucf/Services/FeatureSettingsService/IFeatureSettingsService.h>

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;
using namespace ucf::utilities::screenrecording;

namespace commonHead::viewModels {

// ============================================================================
// Factory
// ============================================================================

std::shared_ptr<IRecordingViewModel> IRecordingViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<RecordingViewModel>(commonHeadFramework);
}

// ============================================================================
// Construction / Destruction
// ============================================================================

RecordingViewModel::RecordingViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : IRecordingViewModel(framework)
{
}

RecordingViewModel::~RecordingViewModel()
{
    // Wait for any pending async stop operation
    if (m_stopThread.joinable()) {
        m_stopThread.join();
    }

    // Ensure timer thread is stopped
    stopDurationTimer();

    // If still recording, force stop
    if (m_session.isValid()) {
        ScreenRecordingUtils::stopRecording(m_session);
    }
}

std::string RecordingViewModel::getViewModelName() const
{
    return "RecordingViewModel";
}

// ============================================================================
// Init
// ============================================================================

void RecordingViewModel::init()
{
    // Discover appDir from the framework's data storage path
    // For FFmpeg discovery we need the app binary directory.
    // The ICommonHeadFramework provides dataStoragePath; appDir is derived
    // at the UI layer and passed via settings. However, for a clean approach,
    // we try to find ffmpeg using common system paths first.
    // The UI layer can also call updateSettings() with the correct appDir.

    // Load settings from FeatureSettingsService if available
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto featureSettingsService = serviceLocator->getFeatureSettingsService().lock())
            {
                auto serviceSettings = featureSettingsService->getRecordingSettings();
                m_settings.outputDirectory = serviceSettings.outputDirectory;
                m_settings.videoFormat = serviceSettings.videoFormat;
                m_settings.framesPerSecond = serviceSettings.framesPerSecond;
            }
        }
    }

    // FFmpeg discovery will be done when appDir is provided via setAppDir
    // or lazily on first startRecording call
}

// ============================================================================
// State Management
// ============================================================================

model::RecordingState RecordingViewModel::getState() const
{
    std::lock_guard lock(m_mutex);
    return m_state;
}

void RecordingViewModel::setState(model::RecordingState newState)
{
    {
        std::lock_guard lock(m_mutex);
        if (m_state == newState) return;
        m_state = newState;
    }
    fireNotification(&IRecordingViewModelCallback::onStateChanged, newState);
}

int RecordingViewModel::getDuration() const
{
    return m_duration.load();
}

bool RecordingViewModel::isFFmpegAvailable() const
{
    std::lock_guard lock(m_mutex);
    return !m_ffmpegPath.empty();
}

std::string RecordingViewModel::getFFmpegPath() const
{
    std::lock_guard lock(m_mutex);
    return m_ffmpegPath;
}

void RecordingViewModel::setAppDir(const std::string& appDir)
{
    std::lock_guard lock(m_mutex);
    m_appDir = appDir;
    m_ffmpegPath = ScreenRecordingUtils::findFFmpegPath(appDir);
}

// ============================================================================
// Recording Control
// ============================================================================

void RecordingViewModel::startRecording(int displayIndex)
{
    {
        std::lock_guard lock(m_mutex);
        if (m_state != model::RecordingState::Idle) {
            fireNotification(&IRecordingViewModelCallback::onError,
                             std::string("Recording already in progress"));
            return;
        }
        if (m_ffmpegPath.empty()) {
            fireNotification(&IRecordingViewModelCallback::onError,
                             std::string("FFmpeg not found. Please install FFmpeg to use recording features."));
            return;
        }
    }

    RecordingConfig config;
    {
        std::lock_guard lock(m_mutex);
        config.ffmpegPath = m_ffmpegPath;
        config.outputPath = generateOutputPath();
        config.videoFormat = m_settings.videoFormat;
        config.fps = m_settings.framesPerSecond;
        config.displayIndex = displayIndex;
        config.isRegion = false;
    }

    doStartRecording(config);
}

void RecordingViewModel::startRegionRecording(int x, int y, int w, int h)
{
    {
        std::lock_guard lock(m_mutex);
        if (m_state != model::RecordingState::Idle) {
            fireNotification(&IRecordingViewModelCallback::onError,
                             std::string("Recording already in progress"));
            return;
        }
        if (m_ffmpegPath.empty()) {
            fireNotification(&IRecordingViewModelCallback::onError,
                             std::string("FFmpeg not found."));
            return;
        }
    }

    RecordingConfig config;
    {
        std::lock_guard lock(m_mutex);
        config.ffmpegPath = m_ffmpegPath;
        config.outputPath = generateOutputPath();
        config.videoFormat = m_settings.videoFormat;
        config.fps = m_settings.framesPerSecond;
        config.isRegion = true;
        config.regionX = x;
        config.regionY = y;
        config.regionW = w;
        config.regionH = h;
    }

    doStartRecording(config);
}

void RecordingViewModel::doStartRecording(const RecordingConfig& config)
{
    // Wait for any pending stop operation to complete
    if (m_stopThread.joinable()) {
        m_stopThread.join();
    }

    auto session = ScreenRecordingUtils::startRecording(config);
    if (!session.isValid()) {
        fireNotification(&IRecordingViewModelCallback::onError,
                         std::string("Failed to start FFmpeg recording process"));
        return;
    }

    {
        std::lock_guard lock(m_mutex);
        m_session = session;
    }

    setState(model::RecordingState::Recording);
    startDurationTimer();
}

void RecordingViewModel::stopRecording()
{
    {
        std::lock_guard lock(m_mutex);
        if (m_state == model::RecordingState::Idle) return;
    }

    // Prevent concurrent stop calls
    bool expected = false;
    if (!m_stopping.compare_exchange_strong(expected, true)) return;

    // Capture whether recording was paused before changing state
    bool wasPaused = false;
    {
        std::lock_guard lock(m_mutex);
        wasPaused = (m_state == model::RecordingState::Paused);
    }

    // Update UI state immediately so the interface stays responsive
    setState(model::RecordingState::Idle);

    // Join any previous stop thread
    if (m_stopThread.joinable()) {
        m_stopThread.join();
    }

    // Run heavy FFmpeg teardown + waitpid on a background thread
    m_stopThread = std::thread([this, wasPaused]() {
        stopDurationTimer();

        RecordingResult result;
        {
            std::lock_guard lock(m_mutex);
            if (m_session.isValid()) {
                if (wasPaused) {
                    ScreenRecordingUtils::resumeRecording(m_session);
                }
                result = ScreenRecordingUtils::stopRecording(m_session);
            }
        }

        if (result.success) {
            fireNotification(&IRecordingViewModelCallback::onRecordingCompleted, result.outputPath);
        } else if (!result.errorMessage.empty()) {
            fireNotification(&IRecordingViewModelCallback::onError, result.errorMessage);
        }

        m_stopping.store(false);
    });
}

void RecordingViewModel::pauseRecording()
{
    std::lock_guard lock(m_mutex);
    if (m_state != model::RecordingState::Recording) return;

    if (ScreenRecordingUtils::pauseRecording(m_session)) {
        m_state = model::RecordingState::Paused;
        // Stop the timer but don't reset
        m_timerRunning.store(false);
        m_timerCv.notify_all();

        fireNotification(&IRecordingViewModelCallback::onStateChanged, model::RecordingState::Paused);
    }
}

void RecordingViewModel::resumeRecording()
{
    std::lock_guard lock(m_mutex);
    if (m_state != model::RecordingState::Paused) return;

    if (ScreenRecordingUtils::resumeRecording(m_session)) {
        m_state = model::RecordingState::Recording;
        // Restart the timer
        m_timerRunning.store(true);
        m_timerCv.notify_all();

        fireNotification(&IRecordingViewModelCallback::onStateChanged, model::RecordingState::Recording);
    }
}

void RecordingViewModel::convertToGif(const std::string& inputPath, const std::string& outputPath)
{
    std::string ffmpeg;
    {
        std::lock_guard lock(m_mutex);
        ffmpeg = m_ffmpegPath;
    }

    std::string outPath = outputPath;
    if (outPath.empty()) {
        // Auto-generate: replace extension with .gif
        auto p = fs::path(inputPath);
        p.replace_extension(".gif");
        outPath = p.string();
    }

    bool ok = ScreenRecordingUtils::convertToGif(ffmpeg, inputPath, outPath, 10);
    if (ok) {
        fireNotification(&IRecordingViewModelCallback::onRecordingCompleted, outPath);
    } else {
        fireNotification(&IRecordingViewModelCallback::onError,
                         std::string("GIF conversion failed"));
    }
}

// ============================================================================
// Settings
// ============================================================================

model::RecordingSettings RecordingViewModel::getSettings() const
{
    std::lock_guard lock(m_mutex);
    return m_settings;
}

void RecordingViewModel::updateSettings(const model::RecordingSettings& settings)
{
    {
        std::lock_guard lock(m_mutex);
        m_settings = settings;
    }
    // Persist to FeatureSettingsService
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto featureSettingsService = serviceLocator->getFeatureSettingsService().lock())
            {
                ucf::service::model::RecordingFeatureSettings serviceSettings;
                serviceSettings.outputDirectory = settings.outputDirectory;
                serviceSettings.videoFormat = settings.videoFormat;
                serviceSettings.framesPerSecond = settings.framesPerSecond;
                featureSettingsService->updateRecordingSettings(serviceSettings);
            }
        }
    }
    fireNotification(&IRecordingViewModelCallback::onSettingsChanged, settings);
}

// ============================================================================
// Internal Helpers
// ============================================================================

std::string RecordingViewModel::generateOutputPath() const
{
    // Format: Recording_YYYYMMDD_HHmmss.ext
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&time, &tm);

    std::ostringstream oss;
    oss << "Recording_"
        << std::put_time(&tm, "%Y%m%d_%H%M%S")
        << "." << m_settings.videoFormat;

    std::string dir = m_settings.outputDirectory;
    if (dir.empty()) {
        dir = "/tmp";
    }

    // Ensure directory exists
    std::error_code ec;
    fs::create_directories(dir, ec);

    return (fs::path(dir) / oss.str()).string();
}

void RecordingViewModel::startDurationTimer()
{
    m_duration.store(0);
    m_timerRunning.store(true);
    m_timerShouldExit.store(false);

    // Stop any existing timer first
    if (m_timerThread.joinable()) {
        m_timerShouldExit.store(true);
        m_timerCv.notify_all();
        m_timerThread.join();
        m_timerRunning.store(true);
        m_timerShouldExit.store(false);
    }

    m_timerThread = std::thread([this]() {
        while (true) {
            std::unique_lock lock(m_timerMutex);
            m_timerCv.wait_for(lock, std::chrono::seconds(1), [this]() {
                return m_timerShouldExit.load();
            });

            if (m_timerShouldExit.load()) {
                break;
            }

            // Only count when actively recording (not paused)
            if (m_timerRunning.load()) {
                int seconds = m_duration.fetch_add(1) + 1;
                fireNotification(&IRecordingViewModelCallback::onDurationChanged, seconds);
            }
        }
    });
}

void RecordingViewModel::stopDurationTimer()
{
    m_timerShouldExit.store(true);
    m_timerRunning.store(false);
    m_timerCv.notify_all();
    if (m_timerThread.joinable()) {
        m_timerThread.join();
    }
}

} // namespace commonHead::viewModels
