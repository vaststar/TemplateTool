#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>
#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h>

namespace ucf::utilities::ffmpeg {

/// Direct libav* (FFmpeg libraries) entry point, as opposed to the CLI flavor
/// that shells out to the ffmpeg/ffprobe executables. Used for in-memory,
/// frame-level work (decode/encode/scale) where spawning a process is unwanted.
///
/// This is a stateless facade for simple, one-shot operations. For each call,
/// a temporary FFmpegSession is created and destroyed internally.
///
/// For more complex workflows (e.g., extracting multiple frames from the same
/// file, or batch operations), use FFmpegSession directly to avoid repeated
/// file opens and codec setup.
///
/// PREFERRED USAGE PATTERNS:
/// - Single operation: FFmpegLib::decodeFirstFrame(path)
/// - Multiple frames: FFmpegSession session; session.open(path);
///                    auto frames = session.extractFramesAt({t1, t2, ...});
class FFmpegLibUtils_EXPORT FFmpegLib
{
public:
    /// Version information of the linked FFmpeg libraries.
    struct Version
    {
        std::string build;     ///< Overall FFmpeg build string (av_version_info).
        std::string avcodec;   ///< e.g. "62.28.100"
        std::string avformat;
        std::string avutil;
    };

    /// Returns the versions of the libav* libraries this binary is linked to.
    /// Also serves as a link/runtime smoke test for the FFmpeg libraries.
    static Version version();

    /// Probe a media file/URL for container and stream metadata (like ffprobe),
    /// without decoding any frames. Returns nullopt on failure.
    static std::optional<MediaInfo> probe(const std::string& url);

    /// Decode the first video frame of @p path (image or media file) into
    /// top-down BGRA using libavformat/libavcodec/libswscale. Returns nullopt
    /// on any failure (missing file, unsupported/corrupt data, no video stream).
    static std::optional<RawImage> decodeFirstFrame(const std::string& path);

    /// Decode the first audio frame of @p path into interleaved PCM. The
    /// current implementation normalizes to S16 while preserving sample rate
    /// and channel count. Returns nullopt on any failure.
    static std::optional<RawAudio> decodeFirstAudioFrame(const std::string& path);

    /// Decode the first audio frame of @p path into interleaved PCM using the
    /// requested output settings. Zero-valued fields in @p output preserve the
    /// input's rate and channel count. Returns nullopt on any failure.
    static std::optional<RawAudio> decodeFirstAudioFrame(const std::string& path,
                                                         const AudioOutputOptions& output);

    /// Seek to @p timestampMs and decode the first audio frame at or after that
    /// point into interleaved PCM. Returns nullopt on any failure.
    static std::optional<RawAudio> extractAudioAt(const std::string& path,
                                                  std::int64_t timestampMs);

    /// Seek to @p timestampMs and decode the first audio frame at or after that
    /// point into interleaved PCM using the requested output settings.
    static std::optional<RawAudio> extractAudioAt(const std::string& path,
                                                  std::int64_t timestampMs,
                                                  const AudioOutputOptions& output);

    /// Seek to @p timestampMs and decode the first video frame at or after that
    /// point into top-down BGRA. Returns nullopt on any failure.
    static std::optional<RawImage> extractFrameAt(const std::string& path,
                                                  std::int64_t timestampMs);

    /// Encode interleaved PCM @p audio and write it as a WAV file to
    /// @p outputPath. Returns false on any failure.
    static bool writeWav(const std::string& outputPath, const RawAudio& audio);

    /// Encode interleaved PCM @p audio and write it as an AAC/ADTS file to
    /// @p outputPath. Returns false on any failure.
    static bool writeAac(const std::string& outputPath, const RawAudio& audio);

};

} // namespace ucf::utilities::ffmpeg


