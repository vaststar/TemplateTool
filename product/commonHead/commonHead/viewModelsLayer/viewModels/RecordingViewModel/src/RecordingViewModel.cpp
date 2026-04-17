#include "RecordingViewModel.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <ucf/Services/FeatureSettingsService/IFeatureSettingsService.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Utilities/TimeUtils/TimeUtils.h>
#include <ucf/Utilities/FilePathUtils/FilePathUtils.h>

#include <chrono>
#include <filesystem>
#include <sstream>

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

    // Auto-discover FFmpeg at startup
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_ffmpegPath = IScreenRecorder::findFFmpegPath();
    }

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
                m_settings.enableMicrophone = serviceSettings.enableMicrophone;
                m_settings.enableSystemAudio = serviceSettings.enableSystemAudio;
                m_settings.micDeviceId = serviceSettings.micDeviceId;
                m_settings.systemAudioDeviceId = serviceSettings.systemAudioDeviceId;
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
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_ffmpegPath.empty();
}

std::string RecordingViewModel::getFFmpegPath() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_ffmpegPath;
}

bool RecordingViewModel::hasScreenRecordingPermission() const
{
    return IScreenRecorder::hasScreenRecordingPermission();
}

// ============================================================================
// Recording Control — delegate to agent
// ============================================================================

void RecordingViewModel::startRecording(int displayIndex)
{
    ucf::agents::RecordingAgentConfig config;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
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
        bool micEffective = m_settings.enableMicrophone && m_agent->hasMicrophonePermission();
        config.audioMode = deriveAudioMode(micEffective, m_settings.enableSystemAudio);
        config.micDevice = m_settings.micDeviceId;
        config.systemAudioDevice = m_settings.systemAudioDeviceId;
        config.systemAudioDeviceType = resolveSystemAudioDeviceType(m_settings.systemAudioDeviceId);
    }
    m_agent->start(config);
}

void RecordingViewModel::startRegionRecording(int x, int y, int w, int h)
{
    ucf::agents::RecordingAgentConfig config;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
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
        bool micEffective = m_settings.enableMicrophone && m_agent->hasMicrophonePermission();
        config.audioMode = deriveAudioMode(micEffective, m_settings.enableSystemAudio);
        config.micDevice = m_settings.micDeviceId;
        config.systemAudioDevice = m_settings.systemAudioDeviceId;
        config.systemAudioDeviceType = resolveSystemAudioDeviceType(m_settings.systemAudioDeviceId);
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
        std::lock_guard<std::mutex> lock(m_mutex);
        ffmpeg = m_ffmpegPath;
    }

    std::string outPath = outputPath;
    if (outPath.empty()) {
        auto p = ucf::utilities::FilePathUtils::pathFromUtf8(inputPath);
        p.replace_extension(".gif");
        outPath = ucf::utilities::FilePathUtils::utf8FromPath(p);
    }

    bool ok = IScreenRecorder::convertToGif(ffmpeg, inputPath, outPath, 10);
    if (ok) {
        fireNotification(&IRecordingViewModelCallback::onRecordingCompleted, outPath);
    } else {
        fireNotification(&IRecordingViewModelCallback::onError,
                         std::string("GIF conversion failed"));
    }
}

void RecordingViewModel::requestThumbnail(const std::string& inputPath)
{
    if (inputPath.empty())
    {
        return;
    }

    const auto readyPath = getThumbnailPath(inputPath);
    if (!readyPath.empty())
    {
        fireNotification(&IRecordingViewModelCallback::onThumbnailReady, inputPath, readyPath);
        return;
    }

    std::string ffmpegPath;
    std::string outputPath;
    {
        std::lock_guard lock(m_mutex);
        if (m_ffmpegPath.empty())
        {
            fireNotification(&IRecordingViewModelCallback::onThumbnailFailed,
                             inputPath,
                             std::string("FFmpeg not found."));
            return;
        }

        if (!m_pendingThumbnailRequests.insert(inputPath).second)
        {
            return;
        }

        ffmpegPath = m_ffmpegPath;
        outputPath = buildThumbnailPath(inputPath);
    }

    std::weak_ptr<RecordingViewModel> weakSelf =
        std::static_pointer_cast<RecordingViewModel>(shared_from_this());
    m_thumbnailThreadPool.submit([weakSelf, inputPath, ffmpegPath, outputPath]() {
        const bool ok = IScreenRecorder::extractThumbnail(
            ffmpegPath,
            inputPath,
            outputPath,
            0.2,
            320,
            180);

        if (auto self = weakSelf.lock())
        {
            {
                std::lock_guard<std::mutex> lock(self->m_mutex);
                self->m_pendingThumbnailRequests.erase(inputPath);
                if (ok)
                {
                    self->m_thumbnailCache[inputPath] = outputPath;
                }
            }

            if (ok)
            {
                self->fireNotification(&IRecordingViewModelCallback::onThumbnailReady,
                                       inputPath,
                                       outputPath);
            }
            else
            {
                self->fireNotification(&IRecordingViewModelCallback::onThumbnailFailed,
                                       inputPath,
                                       std::string("Thumbnail extraction failed"));
            }
        }
    }, ucf::utilities::TaskPriority::Low, "recording-thumbnail");
}

std::string RecordingViewModel::getThumbnailPath(const std::string& inputPath) const
{
    if (inputPath.empty())
    {
        return {};
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto found = m_thumbnailCache.find(inputPath);
        if (found != m_thumbnailCache.end())
        {
            std::error_code ec;
            auto size = std::filesystem::file_size(
                ucf::utilities::FilePathUtils::pathFromUtf8(found->second), ec);
            if (!ec && size > 0)
            {
                return found->second;
            }
        }
    }

    auto outputPath = buildThumbnailPath(inputPath);
    std::error_code ec;
    auto size = std::filesystem::file_size(
        ucf::utilities::FilePathUtils::pathFromUtf8(outputPath), ec);
    if (!ec && size > 0)
    {
        return outputPath;
    }
    return {};
}

// ============================================================================
// Settings
// ============================================================================

model::RecordingSettings RecordingViewModel::getSettings() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_settings;
}

void RecordingViewModel::updateSettings(const model::RecordingSettings& settings)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
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
                serviceSettings.enableMicrophone = settings.enableMicrophone;
                serviceSettings.enableSystemAudio = settings.enableSystemAudio;
                serviceSettings.micDeviceId = settings.micDeviceId;
                serviceSettings.systemAudioDeviceId = settings.systemAudioDeviceId;
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
    std::filesystem::create_directories(ucf::utilities::FilePathUtils::pathFromUtf8(dir), ec);

    return ucf::utilities::FilePathUtils::utf8FromPath(ucf::utilities::FilePathUtils::pathFromUtf8(dir) / filename);
}

ucf::agents::AudioCaptureMode RecordingViewModel::deriveAudioMode(bool mic, bool sys)
{
    if (mic && sys)  return ucf::agents::AudioCaptureMode::MicAndSystem;
    if (mic)         return ucf::agents::AudioCaptureMode::Microphone;
    if (sys)         return ucf::agents::AudioCaptureMode::SystemAudio;
    return ucf::agents::AudioCaptureMode::None;
}

ucf::agents::AudioDeviceType RecordingViewModel::resolveSystemAudioDeviceType(
    const std::string& deviceId) const
{
    if (deviceId.empty())
    {
        return ucf::agents::AudioDeviceType::LoopbackCapture;
    }
    auto agentDevices = m_agent->getAudioDevices();
    for (const auto& dev : agentDevices)
    {
        if (dev.id == deviceId)
        {
            return static_cast<ucf::agents::AudioDeviceType>(dev.deviceType);
        }
    }
    return ucf::agents::AudioDeviceType::LoopbackCapture;
}

std::vector<model::AudioDeviceInfo> RecordingViewModel::getAudioDevices() const
{
    auto agentDevices = m_agent->getAudioDevices();
    std::vector<model::AudioDeviceInfo> result;
    result.reserve(agentDevices.size());
    for (const auto& dev : agentDevices)
    {
        model::AudioDeviceInfo info;
        info.id = dev.id;
        info.displayName = dev.displayName;
        info.isInput = dev.isInput;
        info.deviceType = static_cast<model::AudioDeviceType>(dev.deviceType);
        result.push_back(std::move(info));
    }
    return result;
}

bool RecordingViewModel::hasMicrophonePermission() const
{
    return m_agent->hasMicrophonePermission();
}

void RecordingViewModel::requestMicrophonePermission(std::function<void(bool)> callback)
{
    m_agent->requestMicrophonePermission(std::move(callback));
}

std::string RecordingViewModel::getThumbnailCacheRoot() const
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto clientInfoService = serviceLocator->getClientInfoService().lock())
            {
                return ucf::utilities::FilePathUtils::utf8FromPath(
                    ucf::utilities::FilePathUtils::pathFromUtf8(clientInfoService->getAppCacheStoragePath()) / "recording_thumbnails");
            }
        }
    }

    if (!m_settings.outputDirectory.empty())
    {
        return ucf::utilities::FilePathUtils::utf8FromPath(
            ucf::utilities::FilePathUtils::pathFromUtf8(m_settings.outputDirectory) / ".thumbnail_cache");
    }

    return ucf::utilities::FilePathUtils::utf8FromPath(
        std::filesystem::temp_directory_path() / "TemplateTool" / "recording_thumbnails");
}

std::string RecordingViewModel::buildThumbnailPath(const std::string& inputPath) const
{
    const auto cacheRoot = ucf::utilities::FilePathUtils::pathFromUtf8(getThumbnailCacheRoot());
    std::error_code ec;
    std::filesystem::create_directories(cacheRoot, ec);
    return ucf::utilities::FilePathUtils::utf8FromPath(cacheRoot / (buildThumbnailCacheKey(inputPath) + ".png"));
}

std::string RecordingViewModel::buildThumbnailCacheKey(const std::string& inputPath)
{
    std::error_code ec;
    const auto filePath = ucf::utilities::FilePathUtils::pathFromUtf8(inputPath);
    const auto normalized = ucf::utilities::FilePathUtils::utf8FromPath(
        std::filesystem::absolute(filePath, ec).lexically_normal());

    std::ostringstream keyStream;
    keyStream << normalized;

    ec.clear();
    const auto lastWrite = std::filesystem::last_write_time(filePath, ec);
    if (!ec)
    {
        const auto lastWriteMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            lastWrite.time_since_epoch()).count();
        keyStream << '|' << lastWriteMs;
    }

    ec.clear();
    const auto fileSize = std::filesystem::file_size(filePath, ec);
    if (!ec)
    {
        keyStream << '|' << fileSize;
    }

    std::ostringstream hashed;
    hashed << std::hex << std::hash<std::string>{}(keyStream.str());
    return hashed.str();
}

} // namespace commonHead::viewModels
