#pragma once

#include <string>
#include <functional>

#include <ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h>

namespace ucf::utilities::ffmpeg::core {

/// Centralized error handling for FFmpeg operations
class FFmpegLibUtils_EXPORT ErrorHandler
{
public:
    /// Error policy for handling codec/stream errors
    enum class ErrorPolicy
    {
        /// Throw exception (not used in C-style API, logged instead)
        ThrowException,

        /// Log error and continue processing
        LogAndContinue,

        /// Retry operation (for transient failures)
        RetryOnce,

        /// Skip current frame/chunk and continue
        SkipAndContinue,

        /// Stop processing entirely
        StopProcessing
    };

    // ========== Configuration ==========

    /// Set default error policy for all operations
    static void setDefaultErrorPolicy(ErrorPolicy policy);

    /// Get current default error policy
    static ErrorPolicy getDefaultErrorPolicy();

    /// Set maximum retry attempts for recoverable errors
    static void setMaxRetries(int maxRetries);

    /// Set custom error callback for logging/monitoring
    using ErrorCallback = std::function<void(int errCode, const std::string& errMsg)>;
    static void setErrorCallback(ErrorCallback callback);

    // ========== Error Translation ==========

    /// Convert FFmpeg error code (libav return value) to human-readable message
    static std::string getErrorString(int avErrorCode);

    /// Check if error is recoverable (e.g., EAGAIN)
    static bool isRecoverableError(int avErrorCode);

    /// Check if error is fatal (e.g., corrupt file)
    static bool isFatalError(int avErrorCode);

    // ========== Context ==========

    /// Create/get thread-local error context
    static void clearLastError();

    /// Set last error (used internally)
    static void setLastError(int errCode, const std::string& errMsg);

    /// Get last error code
    static int getLastErrorCode();

    /// Get last error message
    static std::string getLastErrorMessage();
};

} // namespace ucf::utilities::ffmpeg::core
