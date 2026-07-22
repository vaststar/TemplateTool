#include <ucf/Utilities/FFmpegUtils/Core.h>

namespace ucf::utilities::ffmpeg::core {

void initializeCore()
{
    // Initialize error handler
    ErrorHandler::clearLastError();
    ErrorHandler::setDefaultErrorPolicy(ErrorHandler::ErrorPolicy::LogAndContinue);
    ErrorHandler::setMaxRetries(3);
}

void cleanupCore()
{
    // Cleanup resources if needed
    ErrorHandler::clearLastError();
}

} // namespace ucf::utilities::ffmpeg::core
