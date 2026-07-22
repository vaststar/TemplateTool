#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>
#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h>

namespace ucf::utilities::ffmpeg {

/// Stateful session for complex FFmpeg operations (multi-frame extraction,
/// streaming decode/encode). Holds the open file/container context across
/// multiple operations, avoiding repeated demux/codec setup.
///
/// For simple one-shot operations (probe, single frame/audio) prefer
/// FFmpegLib convenience methods, which create/destroy a Session internally.
///
/// Usage:
///   FFmpegSession session;
///   if (!session.open(path)) return;
///   for (auto ts : timestamps) {
///     auto frame = session.extractFrameAt(ts);
///   }
///   session.close();  // optional; dtor closes automatically
class FFmpegLibUtils_EXPORT FFmpegSession
{
public:
    FFmpegSession();
    ~FFmpegSession();

    FFmpegSession(const FFmpegSession&) = delete;
    FFmpegSession& operator=(const FFmpegSession&) = delete;

    // Open a media file. Probes streams and initializes internal state.
    // Returns false on failure (file not found, no streams, etc).
    bool open(const std::string& path);

    // Close the session and release resources. Safe to call multiple times.
    // Also called automatically in destructor.
    void close();

    // Returns whether a file is currently open.
    bool isOpen() const { return mIsOpen; }

    // Get the current file path. Empty string if not open.
    const std::string& path() const { return mPath; }

    // --- Video Operations ---

    /// Returns the best video stream index, or <0 if none found.
    int bestVideoStream() const;

    /// Decode the first video frame into top-down BGRA.
    /// Returns nullopt on failure.
    std::optional<RawImage> extractFirstFrame();

    /// Seek to @p timestampMs and decode the first video frame at or after that
    /// point into top-down BGRA. Returns nullopt on failure.
    std::optional<RawImage> extractFrameAt(std::int64_t timestampMs);

    /// Extract video frames at multiple timestamps in one call. Returns a vector
    /// of the same size as @p timestamps, with nullopt entries for failures.
    std::vector<std::optional<RawImage>> extractFramesAt(
        const std::vector<std::int64_t>& timestamps);

    // --- Audio Operations ---

    /// Returns the first audio stream index, or <0 if none found.
    int bestAudioStream() const;

    /// Decode the first audio frame into interleaved PCM.
    /// Returns nullopt on failure.
    std::optional<RawAudio> extractFirstAudioFrame();

    /// Decode the first audio frame with output options.
    std::optional<RawAudio> extractFirstAudioFrame(const AudioOutputOptions& output);

    /// Seek to @p timestampMs and decode the first audio frame at or after that
    /// point into interleaved PCM. Returns nullopt on failure.
    std::optional<RawAudio> extractAudioAt(std::int64_t timestampMs);

    /// Seek to @p timestampMs and decode the first audio frame with output options.
    std::optional<RawAudio> extractAudioAt(std::int64_t timestampMs,
                                          const AudioOutputOptions& output);

    /// Extract audio frames at multiple timestamps in one call. Returns a vector
    /// of the same size as @p timestamps, with nullopt entries for failures.
    std::vector<std::optional<RawAudio>> extractAudioAt(
        const std::vector<std::int64_t>& timestamps,
        const AudioOutputOptions& output = AudioOutputOptions{});

    // --- Container Metadata ---

    /// Get the media info (streams, durations, etc.) of the open file.
    /// Returns nullopt if no file is open.
    std::optional<std::reference_wrapper<const MediaInfo>> info() const;

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
    bool mIsOpen = false;
    std::string mPath;
};

} // namespace ucf::utilities::ffmpeg
