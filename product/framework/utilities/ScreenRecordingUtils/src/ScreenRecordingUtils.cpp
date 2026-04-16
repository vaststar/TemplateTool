#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#if defined(_WIN32)
#include "ScreenRecorder_Win.h"
#elif defined(__APPLE__)
#include "ScreenRecorder_Mac.h"
#elif defined(__linux__)
#include "ScreenRecorder_Linux.h"
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

} // namespace ucf::utilities::screenrecording
