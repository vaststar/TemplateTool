#pragma once

#include <string>

#include <ucf/Utilities/FFmpegUtils/FFmpegExport.h>

namespace ucf::utilities::ffmpeg {

/// Locates the bundled FFmpeg / FFprobe command-line executables.
///
/// This consolidates the per-platform discovery logic that previously lived in
/// each screen-recording backend and in ScreenCaptureUtils. It searches, in
/// order: paths relative to this shared library (dev + installed layouts),
/// well-known system locations, then the PATH environment variable.
///
/// Resolved paths are cached; discovery only re-runs while the result is empty
/// (i.e. FFmpeg was not found), so a later install is still picked up.
class FFmpegUtils_EXPORT FFmpegLocator
{
public:
    /// Absolute path to the ffmpeg executable, or empty if not found.
    static std::string ffmpegPath();

    /// Absolute path to the ffmpeg executable, searching relative to an explicit
    /// application directory first. Not cached.
    static std::string ffmpegPath(const std::string& appDir);

    /// Absolute path to the ffprobe executable, or empty if not found.
    static std::string ffprobePath();

    /// Convenience: true if ffmpeg was located.
    static bool isAvailable();
};

} // namespace ucf::utilities::ffmpeg
