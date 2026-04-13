#pragma once

#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

namespace ucf::utilities::screenrecording {

class ScreenRecordingUtils_Win final
{
public:
    static std::string findFFmpegPath(const std::string& appDir);
    static bool hasScreenRecordingPermission() { return true; }
    static bool hasMicrophonePermission() { return true; }
    static std::vector<AudioDeviceInfo> enumerateAudioDevices();
    static RecordingSession startRecording(const RecordingConfig& config);
    static RecordingResult stopRecording(RecordingSession& session);
    static bool pauseRecording(const RecordingSession& session);
    static bool resumeRecording(const RecordingSession& session);
    static bool convertToGif(const std::string& ffmpegPath,
                             const std::string& inputPath,
                             const std::string& outputPath,
                             int fps);
};

} // namespace ucf::utilities::screenrecording
