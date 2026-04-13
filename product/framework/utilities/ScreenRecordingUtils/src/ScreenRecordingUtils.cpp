#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

#if defined(_WIN32)
#include "ScreenRecordingUtils_Win.h"
#elif defined(__APPLE__)
#include "ScreenRecordingUtils_Mac.h"
#elif defined(__linux__)
#include "ScreenRecordingUtils_Linux.h"
#endif

namespace ucf::utilities::screenrecording {

std::string ScreenRecordingUtils::findFFmpegPath(const std::string& appDir)
{
#if defined(_WIN32)
    return ScreenRecordingUtils_Win::findFFmpegPath(appDir);
#elif defined(__APPLE__)
    return ScreenRecordingUtils_Mac::findFFmpegPath(appDir);
#elif defined(__linux__)
    return ScreenRecordingUtils_Linux::findFFmpegPath(appDir);
#else
    (void)appDir;
    return {};
#endif
}

bool ScreenRecordingUtils::isFFmpegAvailable(const std::string& appDir)
{
    return !findFFmpegPath(appDir).empty();
}

bool ScreenRecordingUtils::hasScreenRecordingPermission()
{
#if defined(_WIN32)
    return ScreenRecordingUtils_Win::hasScreenRecordingPermission();
#elif defined(__APPLE__)
    return ScreenRecordingUtils_Mac::hasScreenRecordingPermission();
#elif defined(__linux__)
    return ScreenRecordingUtils_Linux::hasScreenRecordingPermission();
#else
    return true;
#endif
}

bool ScreenRecordingUtils::hasMicrophonePermission()
{
#if defined(_WIN32)
    return ScreenRecordingUtils_Win::hasMicrophonePermission();
#elif defined(__APPLE__)
    return ScreenRecordingUtils_Mac::hasMicrophonePermission();
#elif defined(__linux__)
    return ScreenRecordingUtils_Linux::hasMicrophonePermission();
#else
    return true;
#endif
}

void ScreenRecordingUtils::requestMicrophonePermission(std::function<void(bool granted)> callback)
{
#if defined(__APPLE__)
    ScreenRecordingUtils_Mac::requestMicrophonePermission(std::move(callback));
#else
    // Windows / Linux: no permission needed
    if (callback) callback(true);
#endif
}

std::vector<AudioDeviceInfo> ScreenRecordingUtils::enumerateAudioDevices()
{
#if defined(_WIN32)
    return ScreenRecordingUtils_Win::enumerateAudioDevices();
#elif defined(__APPLE__)
    return ScreenRecordingUtils_Mac::enumerateAudioDevices();
#elif defined(__linux__)
    return ScreenRecordingUtils_Linux::enumerateAudioDevices();
#else
    return {};
#endif
}

RecordingSession ScreenRecordingUtils::startRecording(const RecordingConfig& config)
{
#if defined(_WIN32)
    return ScreenRecordingUtils_Win::startRecording(config);
#elif defined(__APPLE__)
    return ScreenRecordingUtils_Mac::startRecording(config);
#elif defined(__linux__)
    return ScreenRecordingUtils_Linux::startRecording(config);
#else
    (void)config;
    return {};
#endif
}

RecordingResult ScreenRecordingUtils::stopRecording(RecordingSession& session)
{
#if defined(_WIN32)
    return ScreenRecordingUtils_Win::stopRecording(session);
#elif defined(__APPLE__)
    return ScreenRecordingUtils_Mac::stopRecording(session);
#elif defined(__linux__)
    return ScreenRecordingUtils_Linux::stopRecording(session);
#else
    (void)session;
    return {false, {}, "Platform not supported"};
#endif
}

bool ScreenRecordingUtils::pauseRecording(const RecordingSession& session)
{
#if defined(_WIN32)
    return ScreenRecordingUtils_Win::pauseRecording(session);
#elif defined(__APPLE__)
    return ScreenRecordingUtils_Mac::pauseRecording(session);
#elif defined(__linux__)
    return ScreenRecordingUtils_Linux::pauseRecording(session);
#else
    (void)session;
    return false;
#endif
}

bool ScreenRecordingUtils::resumeRecording(const RecordingSession& session)
{
#if defined(_WIN32)
    return ScreenRecordingUtils_Win::resumeRecording(session);
#elif defined(__APPLE__)
    return ScreenRecordingUtils_Mac::resumeRecording(session);
#elif defined(__linux__)
    return ScreenRecordingUtils_Linux::resumeRecording(session);
#else
    (void)session;
    return false;
#endif
}

bool ScreenRecordingUtils::convertToGif(const std::string& ffmpegPath,
                                        const std::string& inputPath,
                                        const std::string& outputPath,
                                        int fps)
{
#if defined(_WIN32)
    return ScreenRecordingUtils_Win::convertToGif(ffmpegPath, inputPath, outputPath, fps);
#elif defined(__APPLE__)
    return ScreenRecordingUtils_Mac::convertToGif(ffmpegPath, inputPath, outputPath, fps);
#elif defined(__linux__)
    return ScreenRecordingUtils_Linux::convertToGif(ffmpegPath, inputPath, outputPath, fps);
#else
    (void)ffmpegPath; (void)inputPath; (void)outputPath; (void)fps;
    return false;
#endif
}

} // namespace ucf::utilities::screenrecording
