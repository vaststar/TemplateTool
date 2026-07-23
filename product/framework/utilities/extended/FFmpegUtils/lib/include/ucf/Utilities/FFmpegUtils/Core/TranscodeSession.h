#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>
#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h>

namespace ucf::utilities::ffmpeg::core {

/// High-level transcoding session with progress tracking, multi-track support,
/// and flexible encoding configuration. Runs transcoding in background thread.
class FFmpegLibUtils_EXPORT TranscodeSession
{
public:
    TranscodeSession();
    ~TranscodeSession();

    TranscodeSession(const TranscodeSession&) = delete;
    TranscodeSession& operator=(const TranscodeSession&) = delete;

    // ========== Configuration ==========

    struct VideoConfig
    {
        int width = 0;              // 0 = preserve original
        int height = 0;
        int bitrate = 0;            // kbps, 0 = preserve
        std::string codec;          // e.g. "h264", "hevc", "" = preserve
        float fps = 0.0f;           // 0 = preserve
        std::string filterChain;    // FFmpeg filter chain, e.g. "scale=1280:720,fps=30"
    };

    struct AudioTrackConfig
    {
        int trackIndex = 0;         // Input track index (0-based)
        int sampleRate = 0;         // Hz, 0 = preserve
        int channels = 0;           // 0 = preserve
        int bitrate = 0;            // kbps, 0 = preserve
        std::string codec;          // e.g. "aac", "libmp3lame", "" = preserve
        SampleFormat format = SampleFormat::S16;
    };

    struct TranscodeConfig
    {
        std::string inputPath;
        std::string outputPath;
        std::string outputFormat;   // e.g. "mp4", "mkv", "" = auto-detect from extension

        VideoConfig videoConfig;
        std::vector<AudioTrackConfig> audioTracks;  // Multiple audio tracks

        bool copyVideo = false;     // Skip video transcoding if format matches
        bool copyAudio = false;     // Skip audio transcoding if format matches
        bool includeSubtitles = true;  // Include all subtitle streams
        bool includeData = false;   // Include data streams (e.g., attachments)
    };

    // ========== Operation ==========

    /// Start transcoding with given configuration
    /// @return true if transcoding started successfully (monitor with waitForCompletion)
    bool transcode(const TranscodeConfig& config);

    /// Cancel ongoing transcoding (can be called from another thread)
    void cancel();

    /// Wait for transcoding to complete
    /// @param timeoutMs Maximum time to wait (0 = infinite)
    /// @return true if completed, false if timeout
    bool waitForCompletion(int timeoutMs = 0);

    // ========== Progress & Monitoring ==========

    struct Progress
    {
        float percentage = 0.0f;           // 0.0 to 100.0
        int64_t currentTimeMs = 0;         // Current processing position (ms)
        int64_t totalDurationMs = 0;       // Total duration (ms)
        int64_t bytesWritten = 0;          // Bytes written to output
        int videoFramesProcessed = 0;      // Video frames
        int audioSamplesProcessed = 0;     // Total audio samples

        // Speed metrics
        float encodingSpeed = 0.0f;  // x * real-time (e.g., 2.0x = twice real-time)
        int64_t elapsedSeconds = 0;  // Elapsed time in seconds
    };

    /// Get current progress (can be called from another thread, thread-safe)
    Progress getProgress() const;

    /// Set progress callback (called periodically during transcoding)
    /// Callback runs on transcoding thread, keep it fast
    using ProgressCallback = std::function<void(const Progress& progress)>;
    void setProgressCallback(ProgressCallback callback);

    // ========== Results & Errors ==========

    /// Check if transcoding succeeded
    bool isSuccessful() const;

    /// Get last error message (empty if no error)
    std::string getError() const;

    /// Get output file size (in bytes) after completion
    int64_t getOutputFileSize() const;

private:
    class Impl;  // Pimpl for thread-safe state management
    std::unique_ptr<Impl> mImpl;

    bool doTranscode();
};

using TranscodeSessionPtr = std::shared_ptr<TranscodeSession>;

} // namespace ucf::utilities::ffmpeg::core
