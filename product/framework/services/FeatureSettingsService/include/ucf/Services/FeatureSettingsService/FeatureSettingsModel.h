#pragma once

#include <string>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service::model {

struct SERVICE_EXPORT ScreenshotFeatureSettings {
    std::string outputDirectory;
    std::string imageFormat = "png";     ///< png / jpg / bmp
    int jpegQuality = 90;               ///< 1-100 (JPEG only)
    int captureDelay = 0;               ///< Delay in seconds before capture
    bool addTimestamp = true;           ///< Overlay timestamp watermark
};

struct SERVICE_EXPORT RecordingFeatureSettings {
    std::string outputDirectory;
    std::string videoFormat = "mp4";     ///< mp4 / webm / mov
    int framesPerSecond = 30;            ///< 15 / 30 / 60
    bool enableMicrophone = false;       ///< Record microphone audio
    bool enableSystemAudio = false;      ///< Record system/desktop audio
    std::string micDeviceId;             ///< Selected microphone device ID
    std::string systemAudioDeviceId;     ///< Selected system audio device ID
};

} // namespace ucf::service::model
