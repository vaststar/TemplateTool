
#include "NetworkHttpDownloadToFileHandler.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>
#include <system_error>

#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpTypes.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpRequest.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpResponse.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileRequest.h>
#include <ucf/services/NetworkService/model/HttpDownloadToFileResponse.h>

#include "NetworkHttpTypeConverter.h"
#include "NetworkServiceLogger.h"

namespace ucf::service::network::http{

namespace {

std::optional<std::string_view> findHeader(
    const NetworkHttpHeaders& headers, std::string_view name)
{
    for (const auto& [key, value] : headers) {
        if (key.size() == name.size() &&
            std::equal(key.begin(), key.end(), name.begin(),
                [](char a, char b) {
                    return std::tolower(static_cast<unsigned char>(a)) ==
                           std::tolower(static_cast<unsigned char>(b));
                })) {
            return value;
        }
    }
    return std::nullopt;
}

std::optional<std::uintmax_t> parseRangeStart(
    std::string_view value, std::string_view prefix)
{
    if (!value.starts_with(prefix)) {
        return std::nullopt;
    }
    value.remove_prefix(prefix.size());
    std::uintmax_t start = 0;
    const auto* first = value.data();
    const auto* last = first + value.size();
    const auto [next, error] = std::from_chars(first, last, start);
    if (error != std::errc{} || next == first || next == last || *next != '-') {
        return std::nullopt;
    }
    return start;
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

    int getRedirectCount() const{ return mRedirectCount;}
    void prepareRedirect();

    void convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const;

    bool openFile();
    bool writeToFile(const ByteBuffer& buffer);
    bool closeFile();
private:
    ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc mResponseCallBack;
    ucf::service::network::http::HttpDownloadToFileResponse mDownloadResponse;
    std::string mDownloadFilePath;
    std::ofstream mOutFilestream;
    ucf::infrastructure::network::http::NetworkHttpRequest mHttpRequest;
    int mRedirectCount{0};
};

NetworkHttpDownloadToFileHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc& responseCallback)
    : mResponseCallBack(responseCallback)
{
    convertDownloadRequestToHttpRequest(downloadRequest, mHttpRequest);

    mDownloadFilePath = downloadRequest.getDownloadFilePath();
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

bool NetworkHttpDownloadToFileHandler::DataPrivate::openFile()
{
    if (!mOutFilestream.is_open())
    {
        const int status = mDownloadResponse.getHttpResponseCode();
        std::ios::openmode mode = std::ios::binary | std::ios::trunc;
        if (status == 206) {
            const auto range = findHeader(mHttpRequest.getRequestHeaders(), "Range");
            const auto contentRange = findHeader(mDownloadResponse.getResponseHeaders(), "Content-Range");
            const auto requestedStart = range ? parseRangeStart(*range, "bytes=") : std::nullopt;
            const auto returnedStart = contentRange ? parseRangeStart(*contentRange, "bytes ") : std::nullopt;

            std::error_code ec;
            const auto existingSize = std::filesystem::file_size(mDownloadFilePath, ec);
            if (!requestedStart || !returnedStart ||
                *requestedStart != *returnedStart || ec || existingSize != *requestedStart) {
                mDownloadResponse.setErrorData({0, ResponseErrorType::OtherError,
                    "Invalid Content-Range or partial file size"});
                return false;
            }
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

        // A server may ignore Range and return 200; restart from byte zero.
        mOutFilestream.open(mDownloadFilePath, mode);
        if (!mOutFilestream.is_open()) {
            mDownloadResponse.setErrorData({0, ResponseErrorType::OtherError,
                "Cannot open download file"});
        }
        return mOutFilestream.is_open();
    }
    return true;
}

bool NetworkHttpDownloadToFileHandler::DataPrivate::writeToFile(const ByteBuffer& buffer)
{
    if (!buffer.empty() && openFile())
    {
        mOutFilestream.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        if (!mOutFilestream.good())
        {
            mDownloadResponse.setErrorData({0, ResponseErrorType::OtherError,
                "Cannot write download file"});
            return false;
        }
        mOutFilestream.flush();
        if (!mOutFilestream.good()) {
            mDownloadResponse.setErrorData({0, ResponseErrorType::OtherError,
                "Cannot flush download file"});
        }
        return mOutFilestream.good();
    }
    return false;
}

bool NetworkHttpDownloadToFileHandler::DataPrivate::closeFile()
{
    if (mOutFilestream.is_open())
    {
        mOutFilestream.close();
    }
    return true;
}

void NetworkHttpDownloadToFileHandler::DataPrivate::prepareRedirect()
{
    ++mRedirectCount;
    // Find Location header (case-insensitive)
    const auto& headers = mDownloadResponse.getResponseHeaders();
    auto it = std::find_if(headers.cbegin(), headers.cend(), [](const auto& headerKeyVal){
        constexpr std::string_view location = "Location";
        if (headerKeyVal.first.size() != location.size()) return false;
        return std::equal(headerKeyVal.first.begin(), headerKeyVal.first.end(),
                          location.begin(), location.end(),
                          [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
    });
    if (it != headers.cend())
    {
        mHttpRequest.setRequestUri(it->second);
    }
    
    mDownloadResponse.clear();
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
    
    if (errorData)
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
    mDataPrivate->closeFile();
    mDataPrivate->getDownloadResponse().setFinished();
    if (auto callback = mDataPrivate->getResponseCallback())
    {
        callback(mDataPrivate->getDownloadResponse());
    }
}

bool NetworkHttpDownloadToFileHandler::shouldRedirectRequest() const
{
    if (mDataPrivate->getDownloadResponse().getErrorData().has_value()) {
        return false;
    }
    if (301 == mDataPrivate->getDownloadResponse().getHttpResponseCode() ||
        302 == mDataPrivate->getDownloadResponse().getHttpResponseCode() ||
        303 == mDataPrivate->getDownloadResponse().getHttpResponseCode() ||
        307 == mDataPrivate->getDownloadResponse().getHttpResponseCode() ||
        308 == mDataPrivate->getDownloadResponse().getHttpResponseCode())
    {
        return mDataPrivate->getRedirectCount() < kMaxRedirectCount;
    }
    return false;
}

void NetworkHttpDownloadToFileHandler::prepareRedirectRequest()
{
    if (shouldRedirectRequest())
    {
        mDataPrivate->prepareRedirect();
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpDownloadToFileHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
