#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#include <algorithm>
#include <filesystem>

#include <ucf/Utilities/ProcessBridgeUtils/IProcessBridge.h>

#if defined(_WIN32)
#include "windows/ScreenRecorder_Win.h"
#elif defined(__APPLE__)
#include "macos/ScreenRecorder_Mac.h"
#elif defined(__linux__)
#include "linux/ScreenRecorder_Linux.h"
#endif

namespace ucf::utilities::screenrecording {

// ============================================================================
// IScreenRecorder factory
// ============================================================================

std::unique_ptr<IScreenRecorder> IScreenRecorder::create()
{
#if defined(_WIN32)
    return std::make_unique<ScreenRecorder_Win>();
#elif defined(__APPLE__)
    return std::make_unique<ScreenRecorder_Mac>();
#elif defined(__linux__)
    return std::make_unique<ScreenRecorder_Linux>();
#else
    return nullptr;
#endif
}

// ============================================================================
// IScreenRecorder — static utilities
// ============================================================================

std::string IScreenRecorder::findFFmpegPath()
{
#if defined(_WIN32)
    return ScreenRecorder_Win::findFFmpegPath();
#elif defined(__APPLE__)
    return ScreenRecorder_Mac::findFFmpegPath();
#elif defined(__linux__)
    return ScreenRecorder_Linux::findFFmpegPath();
#else
    return {};
#endif
}

std::string IScreenRecorder::findFFmpegPath(const std::string& appDir)
{
#if defined(_WIN32)
    return ScreenRecorder_Win::findFFmpegPath(appDir);
#elif defined(__APPLE__)
    return ScreenRecorder_Mac::findFFmpegPath(appDir);
#elif defined(__linux__)
    return ScreenRecorder_Linux::findFFmpegPath(appDir);
#else
    (void)appDir;
    return {};
#endif
}

bool IScreenRecorder::isFFmpegAvailable()
{
    return !findFFmpegPath().empty();
}

bool IScreenRecorder::isFFmpegAvailable(const std::string& appDir)
{
    return !findFFmpegPath(appDir).empty();
}

bool IScreenRecorder::hasScreenRecordingPermission()
{
#if defined(_WIN32)
    return true;
#elif defined(__APPLE__)
    return ScreenRecorder_Mac::hasScreenRecordingPermission();
#elif defined(__linux__)
    return true;
#else
    return true;
#endif
}

bool IScreenRecorder::hasMicrophonePermission()
{
#if defined(_WIN32)
    return true;
#elif defined(__APPLE__)
    return ScreenRecorder_Mac::hasMicrophonePermission();
#elif defined(__linux__)
    return true;
#else
    return true;
#endif
}

void IScreenRecorder::requestMicrophonePermission(std::function<void(bool granted)> callback)
{
#if defined(__APPLE__)
    ScreenRecorder_Mac::requestMicrophonePermission(std::move(callback));
#else
    if (callback) callback(true);
#endif
}

std::vector<AudioDeviceInfo> IScreenRecorder::enumerateAudioDevices()
{
#if defined(_WIN32)
    return ScreenRecorder_Win::enumerateAudioDevices();
#elif defined(__APPLE__)
    return ScreenRecorder_Mac::enumerateAudioDevices();
#elif defined(__linux__)
    return ScreenRecorder_Linux::enumerateAudioDevices();
#else
    return {};
#endif
}

bool IScreenRecorder::convertToGif(const std::string& ffmpegPath,
                                   const std::string& inputPath,
                                   const std::string& outputPath,
                                   int fps)
{
#if defined(_WIN32)
    return ScreenRecorder_Win::convertToGif(ffmpegPath, inputPath, outputPath, fps);
#elif defined(__APPLE__)
    return ScreenRecorder_Mac::convertToGif(ffmpegPath, inputPath, outputPath, fps);
#elif defined(__linux__)
    return ScreenRecorder_Linux::convertToGif(ffmpegPath, inputPath, outputPath, fps);
#else
    (void)ffmpegPath; (void)inputPath; (void)outputPath; (void)fps;
    return false;
#endif
}

bool IScreenRecorder::extractThumbnail(const std::string& ffmpegPath,
                                       const std::string& inputPath,
                                       const std::string& outputPath,
                                       double timeSeconds,
                                       int maxWidth,
                                       int maxHeight)
{
    if (ffmpegPath.empty() || inputPath.empty() || outputPath.empty())
    {
        return false;
    }

    maxWidth = std::max(1, maxWidth);
    maxHeight = std::max(1, maxHeight);
    if (timeSeconds < 0.0)
    {
        timeSeconds = 0.0;
    }

    std::error_code ec;
    auto output = std::filesystem::path(outputPath);
    if (!output.parent_path().empty())
    {
        std::filesystem::create_directories(output.parent_path(), ec);
        if (ec)
        {
            return false;
        }
    }

    std::string scaleFilter = "scale=w=" + std::to_string(maxWidth)
        + ":h=" + std::to_string(maxHeight)
        + ":force_original_aspect_ratio=decrease";

    ucf::utilities::ProcessBridgeConfig config;
    config.executablePath = ffmpegPath;
    config.arguments = {
        "-y",
        "-ss", std::to_string(timeSeconds),
        "-i", inputPath,
        "-frames:v", "1",
        "-vf", scaleFilter,
        outputPath
    };
    config.stopTimeoutMs = 30000;

    auto result = ucf::utilities::IProcessBridge::run(config);
    if (result.timedOut || result.exitCode != 0)
    {
        return false;
    }

    auto size = std::filesystem::file_size(output, ec);
    return !ec && size > 0;
}

} // namespace ucf::utilities::screenrecording
