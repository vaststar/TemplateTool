#pragma once

#include <string>
#include <vector>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::viewModels::model {

// ============================================================================
// Recording State
// ============================================================================

enum class RecordingState {
    Idle,           ///< No active recording
    Recording,      ///< Recording in progress
    Paused          ///< Recording paused
};

// ============================================================================
// Audio Device Info (ViewModel-layer mirror of utilities::AudioDeviceInfo)
// ============================================================================

struct COMMONHEAD_EXPORT AudioDeviceInfo {
    std::string id;           ///< Platform-specific device identifier
    std::string displayName;  ///< User-friendly display name
    bool isInput = true;      ///< true = microphone/input, false = output/loopback
};

// ============================================================================
// Recording Settings
// ============================================================================

struct COMMONHEAD_EXPORT RecordingSettings {
    std::string outputDirectory;
    std::string videoFormat = "mp4";     ///< mp4 / webm / mov
    int framesPerSecond = 30;            ///< 15 / 30 / 60
    bool enableMicrophone = false;       ///< Record microphone audio
    bool enableSystemAudio = false;      ///< Record system/desktop audio
    std::string micDeviceId;             ///< Selected microphone device ID
    std::string systemAudioDeviceId;     ///< Selected system audio device ID
};

} // namespace commonHead::viewModels::model
