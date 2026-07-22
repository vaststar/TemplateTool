#pragma once

#include <memory>
#include <string>
#include <optional>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>
#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h>

namespace ucf::utilities::ffmpeg::core {

/// Abstract base class for all media processing elements (decoders, encoders, filters, etc).
/// Provides unified interface for pipeline-based media processing.
class FFmpegLibUtils_EXPORT MediaElement
{
public:
    virtual ~MediaElement() = default;

    /// Element types for runtime identification
    enum class Type
    {
        Demuxer,
        Decoder,
        Encoder,
        Filter,
        Muxer,
        Custom
    };

    /// Processing state
    enum class State
    {
        Closed,      // Not initialized
        Opening,     // In progress
        Open,        // Ready to process
        Processing,  // Actively processing
        Eof,         // End of stream reached
        Error        // Error occurred
    };

    // ========== Lifecycle ==========

    /// Open/initialize the element with a source (file, stream, etc).
    /// @return true if successful, false otherwise (check lastError())
    virtual bool open(const std::string& source) = 0;

    /// Close and release resources. Safe to call multiple times.
    virtual void close() = 0;

    /// Get current state
    virtual State getState() const = 0;

    // ========== Data Processing ==========

    /// Process one frame/chunk of data. Call repeatedly to process entire stream.
    /// @return true if processed, false if EOF or error (check isEof())
    virtual bool process() = 0;

    /// Check if end-of-stream reached
    virtual bool isEof() const = 0;

    // ========== Error Handling ==========

    /// Get last error message (non-empty only if error occurred)
    virtual std::string lastError() const = 0;

    /// Get error code from last operation (0 = success)
    virtual int lastErrorCode() const = 0;

    // ========== Element Info ==========

    /// Get element type
    virtual Type getType() const = 0;

    /// Get human-readable name
    virtual std::string getName() const = 0;

    /// Get element description (e.g., codec name, filter string)
    virtual std::string getDescription() const = 0;
};

using MediaElementPtr = std::shared_ptr<MediaElement>;

} // namespace ucf::utilities::ffmpeg::core
