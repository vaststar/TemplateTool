#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>

// =============================================================================
// FFmpegTypes - neutral data structures shared across the libav-based flavor.
//
// These types intentionally carry NO libav dependency, so they form the common
// layer that the high-level facade (FFmpegLib) and future low-level modules
// (Demuxer / Decoder / Encoder / ...) all exchange. libav headers stay confined
// to the .cpp side (see src/FFmpegInternal.h).
// =============================================================================

namespace ucf::utilities::ffmpeg {

/// Kind of an elementary stream inside a container.
enum class MediaType
{
    Unknown,
    Video,
    Audio,
    Subtitle,
};

/// Metadata about a single elementary stream. Fields not relevant to the
/// stream's type are left at their defaults.
struct StreamInfo
{
    MediaType type = MediaType::Unknown;
    std::string codec;         ///< Codec name, e.g. "h264", "aac", "png".
    std::int64_t bitrate = 0;  ///< Stream bitrate in bits/s, 0 if unknown.
    // Video
    int width = 0;
    int height = 0;
    double fps = 0.0;
    // Audio
    int sampleRate = 0;
    int channels = 0;
};

/// Container-level metadata plus the list of elementary streams.
struct MediaInfo
{
    std::string formatName;      ///< e.g. "mov,mp4,m4a,...", "png_pipe".
    std::int64_t durationMs = 0; ///< Duration in ms, 0 if unknown.
    std::int64_t bitrate = 0;    ///< Overall bitrate in bits/s, 0 if unknown.
    std::vector<StreamInfo> streams;
};

/// Packed pixel layouts that fit a single tightly-packed buffer. Planar formats
/// (I420/NV12) are intentionally not here yet — they need a multi-plane frame
/// type, added when an encoding/GPU path requires them.
enum class PixelFormat
{
    BGRA,   ///< 4 bytes/pixel: B,G,R,A. Desktop display (Qt/Win/macOS).
    RGBA,   ///< 4 bytes/pixel: R,G,B,A. GL textures.
    RGB24,  ///< 3 bytes/pixel: R,G,B.
};

/// Bytes per pixel for a packed PixelFormat.
inline int bytesPerPixel(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::BGRA:
    case PixelFormat::RGBA:  return 4;
    case PixelFormat::RGB24: return 3;
    }
    return 0;
}

/// Packed audio sample layouts that fit a single interleaved buffer.
enum class SampleFormat
{
    S16,    ///< 16-bit signed PCM.
    S32,    ///< 32-bit signed PCM.
    FLT,    ///< 32-bit float PCM.
    DBL,    ///< 64-bit float PCM.
};

/// Bytes per sample for a packed SampleFormat.
inline int bytesPerSample(SampleFormat format)
{
    switch (format)
    {
    case SampleFormat::S16: return 2;
    case SampleFormat::S32: return 4;
    case SampleFormat::FLT: return 4;
    case SampleFormat::DBL: return 8;
    }
    return 0;
}

/// Raw decoded image: tightly packed, top-down. Byte layout is described by
/// @ref format (defaults to BGRA).
struct RawImage
{
    std::vector<std::uint8_t> pixels;
    int width = 0;
    int height = 0;
    PixelFormat format = PixelFormat::BGRA;
};

/// Raw decoded audio: interleaved, tightly packed PCM samples.
struct RawAudio
{
    std::vector<std::uint8_t> samples;
    int sampleRate = 0;
    int channels = 0;
    SampleFormat format = SampleFormat::S16;
};

/// Preferred output settings for audio decode/transcode operations. A zero
/// value means "preserve the input" for that dimension.
struct AudioOutputOptions
{
    SampleFormat format = SampleFormat::S16;
    int sampleRate = 0;
    int channels = 0;
};

/// Preferred settings for writing encoded audio files.
/// Empty format/codec means auto-select by output path extension.
struct AudioEncodeOptions
{
    std::string formatName;
    std::string codecName;
    int bitrate = 128000;
    int sampleRate = 0;
    int channels = 0;
};

} // namespace ucf::utilities::ffmpeg
