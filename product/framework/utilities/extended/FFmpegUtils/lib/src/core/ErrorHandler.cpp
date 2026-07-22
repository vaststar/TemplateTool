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
    ErrorHandler::ErrorPolicy errorPolicy = ErrorHandler::ErrorPolicy::LogAndContinue;
    int maxRetries = 3;
    ErrorHandler::ErrorCallback errorCallback = nullptr;

    int lastErrorCode = 0;
    std::string lastErrorMsg;
    std::mutex mutex;

    static thread_local ErrorContext tls;
};

thread_local ErrorContext ErrorContext::tls;

} // namespace

void ErrorHandler::setDefaultErrorPolicy(ErrorPolicy policy)
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    ErrorContext::tls.errorPolicy = policy;
}

ErrorHandler::ErrorPolicy ErrorHandler::getDefaultErrorPolicy()
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    return ErrorContext::tls.errorPolicy;
}

void ErrorHandler::setMaxRetries(int maxRetries)
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    ErrorContext::tls.maxRetries = maxRetries;
}

void ErrorHandler::setErrorCallback(ErrorCallback callback)
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    ErrorContext::tls.errorCallback = callback;
}

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

void ErrorHandler::clearLastError()
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    ErrorContext::tls.lastErrorCode = 0;
    ErrorContext::tls.lastErrorMsg.clear();
}

void ErrorHandler::setLastError(int errCode, const std::string& errMsg)
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    ErrorContext::tls.lastErrorCode = errCode;
    ErrorContext::tls.lastErrorMsg = errMsg;

    if (ErrorContext::tls.errorCallback)
    {
        ErrorContext::tls.errorCallback(errCode, errMsg);
    }
}

int ErrorHandler::getLastErrorCode()
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    return ErrorContext::tls.lastErrorCode;
}

std::string ErrorHandler::getLastErrorMessage()
{
    std::lock_guard<std::mutex> lock(ErrorContext::tls.mutex);
    return ErrorContext::tls.lastErrorMsg;
}

} // namespace ucf::utilities::ffmpeg::core
