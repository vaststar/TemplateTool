#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#include <ucf/Utilities/FFmpegUtils/Cli/FFmpegCli.h>
#include <ucf/Utilities/FFmpegUtils/Cli/FFmpegLocator.h>

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
    return ffmpeg::FFmpegLocator::ffmpegPath();
}

bool IScreenRecorder::isFFmpegAvailable()
{
    return !findFFmpegPath().empty();
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
    return ffmpeg::FFmpegCli::convertToGif(inputPath, outputPath, fps, ffmpegPath);
}

bool IScreenRecorder::extractThumbnail(const std::string& ffmpegPath,
                                       const std::string& inputPath,
                                       const std::string& outputPath,
                                       double timeSeconds,
                                       int maxWidth,
                                       int maxHeight)
{
    return ffmpeg::FFmpegCli::extractThumbnail(
        inputPath, outputPath, timeSeconds, maxWidth, maxHeight, ffmpegPath);
}

} // namespace ucf::utilities::screenrecording
