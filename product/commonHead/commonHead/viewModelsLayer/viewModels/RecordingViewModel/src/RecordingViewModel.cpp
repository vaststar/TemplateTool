#include "RecordingViewModel.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <ucf/Services/FeatureSettingsService/IFeatureSettingsService.h>
#include <ucf/Utilities/TimeUtils/TimeUtils.h>

#include <filesystem>

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
    , m_agent(ucf::agents::IScreenRecordingAgent::create())
{
}

RecordingViewModel::~RecordingViewModel()
{
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
    m_agent->registerCallback(shared_from_this());

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
}

// ============================================================================
// State Query
// ============================================================================

model::RecordingState RecordingViewModel::getState() const
{
    if (m_agent->isPaused()) return model::RecordingState::Paused;
    if (m_agent->isRecording()) return model::RecordingState::Recording;
    return model::RecordingState::Idle;
}

int RecordingViewModel::getDuration() const
{
    return m_agent->duration();
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

bool RecordingViewModel::hasScreenRecordingPermission() const
{
    return ScreenRecordingUtils::hasScreenRecordingPermission();
}

// ============================================================================
// Recording Control — delegate to agent
// ============================================================================

void RecordingViewModel::startRecording(int displayIndex)
{
    ucf::agents::RecordingAgentConfig config;
    {
        std::lock_guard lock(m_mutex);
        if (m_ffmpegPath.empty()) {
            fireNotification(&IRecordingViewModelCallback::onError,
                             std::string("FFmpeg not found. Please install FFmpeg to use recording features."));
            return;
        }
        config.ffmpegPath = m_ffmpegPath;
        config.outputPath = generateOutputPath();
        config.videoFormat = m_settings.videoFormat;
        config.fps = m_settings.framesPerSecond;
        config.displayIndex = displayIndex;
        config.isRegion = false;
    }
    m_agent->start(config);
}

void RecordingViewModel::startRegionRecording(int x, int y, int w, int h)
{
    ucf::agents::RecordingAgentConfig config;
    {
        std::lock_guard lock(m_mutex);
        if (m_ffmpegPath.empty()) {
            fireNotification(&IRecordingViewModelCallback::onError,
                             std::string("FFmpeg not found."));
            return;
        }
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
    m_agent->start(config);
}

void RecordingViewModel::stopRecording()
{
    m_agent->stop();
}

void RecordingViewModel::abortRecording()
{
    m_agent->abort();
}

void RecordingViewModel::pauseRecording()
{
    m_agent->pause();
}

void RecordingViewModel::resumeRecording()
{
    m_agent->resume();
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
// Agent Callback Translation → IRecordingViewModelCallback
// ============================================================================

void RecordingViewModel::onAgentStateChanged(ucf::agents::RecordingAgentState /*state*/)
{
    // State changes are currently translated via the specific callbacks below.
    // This hook is available for future use (e.g. exposing Starting/Stopping
    // to the UI layer).
}

void RecordingViewModel::onRecordingStarted()
{
    fireNotification(&IRecordingViewModelCallback::onStateChanged, model::RecordingState::Recording);
}

void RecordingViewModel::onRecordingPaused()
{
    fireNotification(&IRecordingViewModelCallback::onStateChanged, model::RecordingState::Paused);
}

void RecordingViewModel::onRecordingResumed()
{
    fireNotification(&IRecordingViewModelCallback::onStateChanged, model::RecordingState::Recording);
}

void RecordingViewModel::onDurationChanged(int seconds)
{
    fireNotification(&IRecordingViewModelCallback::onDurationChanged, seconds);
}

void RecordingViewModel::onRecordingCompleted(const std::string& outputPath)
{
    fireNotification(&IRecordingViewModelCallback::onStateChanged, model::RecordingState::Idle);
    fireNotification(&IRecordingViewModelCallback::onRecordingCompleted, outputPath);
}

void RecordingViewModel::onRecordingAborted()
{
    fireNotification(&IRecordingViewModelCallback::onStateChanged, model::RecordingState::Idle);
}

void RecordingViewModel::onError(const std::string& message)
{
    fireNotification(&IRecordingViewModelCallback::onStateChanged, model::RecordingState::Idle);
    fireNotification(&IRecordingViewModelCallback::onError, message);
}

// ============================================================================
// Internal Helpers
// ============================================================================

std::string RecordingViewModel::generateOutputPath() const
{
    auto timestamp = ucf::utilities::TimeUtils::formatCurrentLocalTime("%Y%m%d_%H%M%S");
    std::string filename = "Recording_" + timestamp + "." + m_settings.videoFormat;

    std::string dir = m_settings.outputDirectory;
    if (dir.empty()) {
        dir = "/tmp";
    }

    std::error_code ec;
    fs::create_directories(dir, ec);

    return (fs::path(dir) / filename).string();
}

} // namespace commonHead::viewModels
