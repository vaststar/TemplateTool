#include <ucf/Utilities/FFmpegUtils/Core/ErrorHandler.h>

extern "C" {
#include <libavutil/error.h>
}

#include <thread>
#include <mutex>

namespace ucf::utilities::ffmpeg::core {

namespace {

class ErrorContext {
public:
    int lastErrorCode = 0;
    std::string lastErrorMsg;
    std::mutex mutex;

    static thread_local ErrorContext tls;
};

thread_local ErrorContext ErrorContext::tls;

} // namespace

std::string ErrorHandler::getErrorString(int avErrorCode)
{
    if (avErrorCode == 0)
    {
        return "Success";
    }

    char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(avErrorCode, errBuf, sizeof(errBuf));
    return std::string(errBuf);
}

bool ErrorHandler::isRecoverableError(int avErrorCode)
{
    // EAGAIN = needs more data
    // AVERROR_EOF = end of stream
    return avErrorCode == AVERROR(EAGAIN) || avErrorCode == AVERROR_EOF;
}

bool ErrorHandler::isFatalError(int avErrorCode)
{
    // Negative values < -1000 are typically fatal
    return avErrorCode < AVERROR(EINVAL);
}

std::optional<ErrorHandler::ErrorInfo> ErrorHandler::getLastError()
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);

    if (ErrorContext::tls.lastErrorCode == 0)
    {
        return std::nullopt;
    }

    return ErrorInfo{
        ErrorContext::tls.lastErrorCode,
        ErrorContext::tls.lastErrorMsg
    };
}

void ErrorHandler::setLastError(int errCode, const std::string& errMsg)
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    ErrorContext::tls.lastErrorCode = errCode;
    ErrorContext::tls.lastErrorMsg = errMsg;
}

void ErrorHandler::clearLastError()
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    ErrorContext::tls.lastErrorCode = 0;
    ErrorContext::tls.lastErrorMsg.clear();
}

} // namespace ucf::utilities::ffmpeg::core
