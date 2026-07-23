#include <ucf/Utilities/FFmpegUtils/Core.h>

namespace ucf::utilities::ffmpeg::core {

void initializeCore()
{
    // Initialize error handler - clear any previous errors
    ErrorHandler::clearLastError();
    // Additional framework initialization can be added here as needed
}

void cleanupCore()
{
    // Cleanup resources if needed
    ErrorHandler::clearLastError();
}

} // namespace ucf::utilities::ffmpeg::core
