#pragma once

#include <string>
#include <vector>

#include <ucf/Agents/AgentsCommonFile/AgentsExport.h>
#include <ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h>

namespace ucf::agents {

/// Current state of the Screen Recording Agent.
enum class RecordingAgentState
{
    Idle,        ///< Not started or finished
    Starting,    ///< FFmpeg process launching
    Recording,   ///< Actively recording
    Paused,      ///< Recording paused (FFmpeg suspended)
    Stopping     ///< FFmpeg shutting down
};

/// Re-export audio types from utilities layer for convenience.
using AudioCaptureMode = ucf::utilities::screenrecording::AudioCaptureMode;
using AudioDeviceInfo = ucf::utilities::screenrecording::AudioDeviceInfo;

/// Configuration for starting a screen recording session.
struct Agents_EXPORT RecordingAgentConfig
{
    std::string ffmpegPath;             ///< Absolute path to ffmpeg binary
    std::string outputPath;             ///< Full output file path (e.g. /path/recording.mp4)
    std::string videoFormat = "mp4";    ///< mp4 / mov / webm
    int fps = 30;                       ///< Frames per second
    int displayIndex = 0;               ///< Display to capture (fullscreen)
    // Region capture
    int regionX = 0, regionY = 0;
    int regionW = 0, regionH = 0;
    bool isRegion = false;              ///< true = region capture, false = fullscreen
    // Audio capture
    AudioCaptureMode audioMode = AudioCaptureMode::None;
    std::string micDevice;              ///< Microphone device id (empty = system default)
    std::string systemAudioDevice;      ///< System audio device id
};

} // namespace ucf::agents
