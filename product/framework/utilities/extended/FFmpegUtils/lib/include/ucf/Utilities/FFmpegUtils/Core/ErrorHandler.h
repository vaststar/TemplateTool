#pragma once

#include <string>
#include <optional>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>

namespace ucf::utilities::ffmpeg::core {

/// Centralized error handling for FFmpeg operations with thread-local context
class FFmpegLibUtils_EXPORT ErrorHandler
{
public:
    struct ErrorInfo {
        int code;           // FFmpeg error code
        std::string message;  // Human-readable message
    };

    // ========== Error Translation ==========

    /// Convert FFmpeg error code to human-readable message
    static std::string getErrorString(int avErrorCode);

    /// Check if error is recoverable (e.g., EAGAIN, EOF)
    static bool isRecoverableError(int avErrorCode);

    /// Check if error is fatal (e.g., corrupt file)
    static bool isFatalError(int avErrorCode);

    // ========== Thread-Local Error Context ==========

    /// Get last error (thread-safe, returns nullopt if no error)
    static std::optional<ErrorInfo> getLastError();

    /// Set last error (used internally)
    static void setLastError(int errCode, const std::string& errMsg);

    /// Clear last error
    static void clearLastError();
};

} // namespace ucf::utilities::ffmpeg::core
