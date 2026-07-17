#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <ucf/Utilities/FFmpegUtils/Cli/FFmpegExport.h>

namespace ucf::utilities::ffmpeg {

/// Stateless wrappers around one-shot FFmpeg command-line invocations.
///
/// Every method runs the bundled ffmpeg/ffprobe executable as a child process
/// through ProcessBridge (argument-vector based, never a shell string), so
/// there is no command-injection surface. When an executable path is not
/// supplied it is resolved via FFmpegLocator.
class FFmpegUtils_EXPORT FFmpegCli
{
public:
    /// Raw decoded image: tightly packed, top-down BGRA (4 bytes/pixel).
    struct RawImage
    {
        std::vector<std::uint8_t> pixels;
        int width = 0;
        int height = 0;
    };

    /// Transcode a video file to an optimized GIF (palettegen/paletteuse).
    /// @param ffmpegPath  optional explicit ffmpeg path; empty = auto-locate.
    /// @return true on success and a non-empty output file.
    static bool convertToGif(const std::string& inputPath,
                             const std::string& outputPath,
                             int fps = 10,
                             const std::string& ffmpegPath = {});

    /// Extract a single representative frame as an image, scaled to fit within
    /// maxWidth x maxHeight while preserving aspect ratio.
    /// @param ffmpegPath  optional explicit ffmpeg path; empty = auto-locate.
    static bool extractThumbnail(const std::string& inputPath,
                                 const std::string& outputPath,
                                 double timeSeconds = 0.2,
                                 int maxWidth = 320,
                                 int maxHeight = 180,
                                 const std::string& ffmpegPath = {});

    /// Decode an image file (e.g. PNG) into top-down BGRA raw pixels.
    /// Dimensions are probed with ffprobe; pixels are produced by ffmpeg.
    /// @param ffmpegPath   optional explicit ffmpeg path; empty = auto-locate.
    /// @param ffprobePath  optional explicit ffprobe path; empty = auto-locate.
    /// @return true on success with out fully populated.
    static bool decodeToBgra(const std::string& imagePath,
                             RawImage& out,
                             const std::string& ffmpegPath = {},
                             const std::string& ffprobePath = {});
};

} // namespace ucf::utilities::ffmpeg
