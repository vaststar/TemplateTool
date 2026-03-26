#pragma once

#include <string>
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
// Recording Settings
// ============================================================================

struct COMMONHEAD_EXPORT RecordingSettings {
    std::string outputDirectory;
    std::string videoFormat = "mp4";     ///< mp4 / webm / mov
    int framesPerSecond = 30;            ///< 15 / 30 / 60
};

} // namespace commonHead::viewModels::model
