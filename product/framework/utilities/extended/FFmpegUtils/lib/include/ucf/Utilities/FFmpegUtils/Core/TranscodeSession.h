#pragma once

#include <cstdint>
#include <string>
#include <functional>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>
#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h>

namespace ucf::utilities::ffmpeg::core {

/// High-level transcoding session combining decoding, filtering, and encoding.
/// Provides progress tracking, cancellation, and comprehensive error handling.
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
    };

    struct AudioConfig
    {
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
        AudioConfig audioConfig;

        bool copyVideo = false;     // Skip video transcoding if format matches
        bool copyAudio = false;     // Skip audio transcoding if format matches
        bool copySubtitles = true;  // Include subtitle streams
    };

    // ========== Operation ==========

    /// Start transcoding with given configuration
    /// @return true if transcoding completed successfully
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
        float percentage;           // 0.0 to 100.0
        int64_t currentTimeMs;      // Current processing position
        int64_t totalDurationMs;    // Total duration (if available)
        int64_t processedBytes;     // Bytes written to output

        int framesProcessed;
        int audioFramesProcessed;
    };

    /// Get current progress (can be called from another thread)
    Progress getProgress() const;

    /// Set progress callback (called periodically during transcoding)
    using ProgressCallback = std::function<void(const Progress& progress)>;
    void setProgressCallback(ProgressCallback callback);

    // ========== Results & Errors ==========

    /// Check if transcoding succeeded
    bool isSuccessful() const;

    /// Get last error message
    std::string getError() const;

    /// Get output file size (in bytes) after completion
    int64_t getOutputFileSize() const;

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};

using TranscodeSessionPtr = std::shared_ptr<TranscodeSession>;

} // namespace ucf::utilities::ffmpeg::core
