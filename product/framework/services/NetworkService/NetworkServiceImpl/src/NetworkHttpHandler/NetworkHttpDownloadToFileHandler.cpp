
#include "NetworkHttpDownloadToFileHandler.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <system_error>

#include <ucf/infrastructure/NetworkClient/LibCurlClient/HttpRedirect.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpTypes.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpRequest.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpResponse.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileRequest.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileResponse.h>
#include <ucf/utilities/FilePathUtils/FilePathUtils.h>

#include "NetworkHttpTypeConverter.h"
#include "NetworkServiceLogger.h"

namespace ucf::service::network::http{

namespace {

bool equalIgnoreCase(std::string_view left, std::string_view right)
{
    return std::equal(left.begin(), left.end(), right.begin(), right.end(), [](char a, char b) {
        return std::tolower(static_cast<unsigned char>(a)) ==
               std::tolower(static_cast<unsigned char>(b));
    });
}

std::optional<std::string_view> findHeader(
    const NetworkHttpHeaders& headers, std::string_view name)
{
    for (const auto& [key, value] : headers) {
        if (equalIgnoreCase(key, name)) {
            return value;
        }
    }
    return std::nullopt;
}

std::optional<std::uintmax_t> consumeRangeNumber(std::string_view& value)
{
    if (value.empty()) {
        return std::nullopt;
    }
    std::uintmax_t number = 0;
    const auto* first = value.data();
    const auto [next, error] = std::from_chars(first, first + value.size(), number);
    if (error != std::errc{}) {
        return std::nullopt;
    }
    value.remove_prefix(next - first);
    return number;
}

std::optional<std::uintmax_t> consumeRangeStart(
    std::string_view& value, std::string_view prefix)
{
    if (!equalIgnoreCase(value.substr(0, prefix.size()), prefix)) {
        return std::nullopt;
    }
    value.remove_prefix(prefix.size());
    const auto start = consumeRangeNumber(value);
    if (!start || !value.starts_with('-')) {
        return std::nullopt;
    }
    value.remove_prefix(1);
    return start;
}

struct ContentRange
{
    std::uintmax_t start;
    std::uintmax_t length;
};

std::optional<ContentRange> parseContentRange(std::string_view value)
{
    const auto start = consumeRangeStart(value, "bytes ");
    const auto end = consumeRangeNumber(value);
    if (!start || !end || *end < *start || !value.starts_with('/')) {
        return std::nullopt;
    }
    value.remove_prefix(1);
    if (value != "*") {
        const auto total = consumeRangeNumber(value);
        if (!total || !value.empty() || *total <= *end) {
            return std::nullopt;
        }
    }

    const auto distance = *end - *start;
    if (distance == std::numeric_limits<std::uintmax_t>::max()) {
        return std::nullopt;
    }
    return ContentRange{*start, distance + 1};
}

} // namespace
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpDownloadToFileHandler::DataPrivate{
public:
    DataPrivate(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const HttpDownloadToFileResponseCallbackFunc& responseCallback);
    
    const ucf::infrastructure::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::service::network::http::HttpDownloadToFileResponse& getDownloadResponse(){return mDownloadResponse;}
    const ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc& getResponseCallback() const{return mResponseCallBack;}

    bool tryPrepareRedirectRequest();

    void convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const;

    bool prepareFileWrite(std::uintmax_t chunkSize);
    bool writeToFile(const ByteBuffer& buffer);
    [[noreturn]] void failFileWrite(const char* message);
    void finishDownload();
private:
    ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc mResponseCallBack;
    ucf::service::network::http::HttpDownloadToFileResponse mDownloadResponse;
    std::filesystem::path mDownloadFilePath;
    std::optional<std::uintmax_t> mRemainingBytes;
    std::optional<std::uintmax_t> mRangeLength;
    std::ofstream mOutFilestream;
    ucf::infrastructure::network::http::NetworkHttpRequest mHttpRequest;
    int mRedirectCount{0};
};

NetworkHttpDownloadToFileHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc& responseCallback)
    : mResponseCallBack(responseCallback)
    , mDownloadFilePath(ucf::utilities::FilePathUtils::pathFromUtf8(downloadRequest.getDownloadFilePath()))
    , mRemainingBytes(downloadRequest.getMaxFileSize())
{
    convertDownloadRequestToHttpRequest(downloadRequest, mHttpRequest);
}

void NetworkHttpDownloadToFileHandler::DataPrivate::convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const
{
    httpRequest.setRequestMethod(ucf::infrastructure::network::http::HTTPMethod::GET);
    httpRequest.setRequestHeaders(downloadRequest.getRequestHeaders());
    httpRequest.setRequestId(downloadRequest.getRequestId());
    httpRequest.setTrackingId(downloadRequest.getTrackingId());
    httpRequest.setRequestUri(downloadRequest.getRequestUri());
    httpRequest.setTimeout(downloadRequest.getTimeout());
}

bool NetworkHttpDownloadToFileHandler::DataPrivate::prepareFileWrite(std::uintmax_t chunkSize)
{
    const bool needsOpen = !mOutFilestream.is_open();
    auto mode = std::ios::binary | std::ios::trunc;
    std::uintmax_t existingBytes = 0;
    if (needsOpen)
    {
        const int status = mDownloadResponse.getHttpResponseCode();
        if (status == 206) {
            auto range = findHeader(mHttpRequest.getRequestHeaders(), "Range");
            const auto contentRange = findHeader(mDownloadResponse.getResponseHeaders(), "Content-Range");
            const auto requestedStart = range ? consumeRangeStart(*range, "bytes=") : std::nullopt;
            const auto returnedRange = contentRange ? parseContentRange(*contentRange) : std::nullopt;

            std::error_code ec;
            const auto existingSize = std::filesystem::file_size(mDownloadFilePath, ec);
            if (!requestedStart || !returnedRange ||
                *requestedStart != returnedRange->start || ec || existingSize != *requestedStart) {
                failFileWrite("Invalid Content-Range or partial file size");
            }
            mRangeLength = returnedRange->length;
            existingBytes = existingSize;
            mode = std::ios::binary | std::ios::app;
        } else if (status >= 300 && status < 400) {
            // Redirect response bodies are not package bytes. The handler will
            // follow Location after this response completes.
            return false;
        } else if (status != 200) {
            mDownloadResponse.setErrorData({status, ResponseErrorType::OtherError,
                "Unexpected download HTTP status"});
            return false;
        }
    }

    if (mRemainingBytes)
    {
        if (existingBytes > *mRemainingBytes || chunkSize > *mRemainingBytes - existingBytes)
        {
            failFileWrite("Download exceeds expected file size");
        }
        *mRemainingBytes -= existingBytes;
    }
    if (mRangeLength && chunkSize > *mRangeLength - mDownloadResponse.getCurrentSize())
    {
        failFileWrite("Download exceeds Content-Range length");
    }

    if (needsOpen)
    {
        // A server may ignore Range and return 200; restart from byte zero.
        mOutFilestream.open(mDownloadFilePath, mode);
        if (!mOutFilestream.is_open())
        {
            failFileWrite("Cannot open download file");
        }
    }
    return true;
}

bool NetworkHttpDownloadToFileHandler::DataPrivate::writeToFile(const ByteBuffer& buffer)
{
    if (!buffer.empty() && prepareFileWrite(buffer.size()))
    {
        mOutFilestream.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        if (!mOutFilestream.good())
        {
            failFileWrite("Cannot write download file");
        }
        mOutFilestream.flush();
        if (!mOutFilestream.good())
        {
            failFileWrite("Cannot flush download file");
        }
        if (mRemainingBytes)
        {
            *mRemainingBytes -= buffer.size();
        }
        return true;
    }
    return false;
}

void NetworkHttpDownloadToFileHandler::DataPrivate::failFileWrite(const char* message)
{
    mDownloadResponse.setErrorData({0, ResponseErrorType::OtherError, message});
    // The curl write callback aborts the transfer when the body sink throws.
    throw std::runtime_error(message);
}

void NetworkHttpDownloadToFileHandler::DataPrivate::finishDownload()
{
    if (mOutFilestream.is_open())
    {
        mOutFilestream.close();
    }
    if (mDownloadResponse.getHttpResponseCode() == 206 && !mDownloadResponse.getErrorData() &&
        (!mRangeLength || mDownloadResponse.getCurrentSize() != *mRangeLength))
    {
        mDownloadResponse.setErrorData({0, ResponseErrorType::OtherError,
            "Download ended before Content-Range was complete"});
    }
}

bool NetworkHttpDownloadToFileHandler::DataPrivate::tryPrepareRedirectRequest()
{
    if (mDownloadResponse.getErrorData().has_value() || mRedirectCount >= kMaxRedirectCount)
    {
        return false;
    }
    if (!ucf::infrastructure::network::libcurl::tryApplyHttpRedirect(
            mHttpRequest, mDownloadResponse.getHttpResponseCode(),
            mDownloadResponse.getResponseHeaders()))
    {
        return false;
    }

    ++mRedirectCount;
    mDownloadResponse.clear();
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpDownloadToFileHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpDownloadToFileHandler::NetworkHttpDownloadToFileHandler(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const HttpDownloadToFileResponseCallbackFunc& restResponseCallback)
    : mDataPrivate(std::make_unique<NetworkHttpDownloadToFileHandler::DataPrivate>(downloadRequest, restResponseCallback))
{

}

NetworkHttpDownloadToFileHandler::~NetworkHttpDownloadToFileHandler()
{

}

const ucf::infrastructure::network::http::NetworkHttpRequest& NetworkHttpDownloadToFileHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpDownloadToFileHandler::setResponseHeader(int statusCode, const ucf::infrastructure::network::http::NetworkHttpHeaders& headers, std::optional<ucf::infrastructure::network::http::ResponseErrorStruct> errorData)
{
    mDataPrivate->getDownloadResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getDownloadResponse().setResponseHeaders(headers);
    
    if (errorData && !mDataPrivate->getDownloadResponse().getErrorData().has_value())
    {
        mDataPrivate->getDownloadResponse().setErrorData(convertToServiceErrorStruct(*errorData));
    }
}

void NetworkHttpDownloadToFileHandler::appendResponseBody(const ucf::infrastructure::network::http::ByteBuffer& buffer, bool isFinished)
{
    const int status = mDataPrivate->getDownloadResponse().getHttpResponseCode();
    if (status >= 300 && status < 400) {
        // Redirect bodies do not belong in the destination file or its
        // progress count; the completed response handles Location.
        return;
    }
    if (!buffer.empty())
    {
        if (mDataPrivate->writeToFile(buffer))
        {
            mDataPrivate->getDownloadResponse().appendResponseBody(buffer);
        }
        else
        {
            SERVICE_LOG_DEBUG("write to file failed");
        }
        
        if (auto callback = mDataPrivate->getResponseCallback())
        {
            callback(mDataPrivate->getDownloadResponse());
        }
    }
}

void NetworkHttpDownloadToFileHandler::completeResponse(const ucf::infrastructure::network::http::HttpResponseMetrics& metrics)
{
    mDataPrivate->finishDownload();
    mDataPrivate->getDownloadResponse().setFinished();
    if (auto callback = mDataPrivate->getResponseCallback())
    {
        callback(mDataPrivate->getDownloadResponse());
    }
}

bool NetworkHttpDownloadToFileHandler::tryPrepareRedirectRequest()
{
    return mDataPrivate->tryPrepareRedirectRequest();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpDownloadToFileHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
