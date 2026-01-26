
#include "NetworkHttpDownloadToFileHandler.h"

#include <algorithm>
#include <cctype>
#include <fstream>

#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToFileRequest.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToFileResponse.h>

#include "NetworkHttpTypeConverter.h"
#include "NetworkServiceLogger.h"

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpDownloadToFileHandler::DataPrivate{
public:
    DataPrivate(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const HttpDownloadToFileResponseCallbackFunc& responseCallback);
    
    const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::service::network::http::HttpDownloadToFileResponse& getDownloadResponse(){return mDownloadResponse;}
    const ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc& getResponseCallback() const{return mResponseCallBack;}

    int getRedirectCount() const{ return mRedirectCount;}
    void prepareRedirect();

    void convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const;

    bool openFile();
    bool writeToFile(const ByteBuffer& buffer);
    bool closeFile();
private:
    ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc mResponseCallBack;
    ucf::service::network::http::HttpDownloadToFileResponse mDownloadResponse;
    std::string mDownloadFilePath;
    std::ofstream mOutFilestream;
    ucf::utilities::network::http::NetworkHttpRequest mHttpRequest;
    int mRedirectCount{0};
};

NetworkHttpDownloadToFileHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc& responseCallback)
    : mResponseCallBack(responseCallback)
{
    convertDownloadRequestToHttpRequest(downloadRequest, mHttpRequest);

    mDownloadFilePath = downloadRequest.getDownloadFilePath();
}

void NetworkHttpDownloadToFileHandler::DataPrivate::convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const
{
    httpRequest.setRequestMethod(ucf::utilities::network::http::HTTPMethod::GET);
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
        mOutFilestream.open(mDownloadFilePath, std::ofstream::binary);
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
            return false;
        }
        mOutFilestream.flush();
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

const ucf::utilities::network::http::NetworkHttpRequest& NetworkHttpDownloadToFileHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpDownloadToFileHandler::setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData)
{
    mDataPrivate->getDownloadResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getDownloadResponse().setResponseHeaders(headers);
    
    if (errorData)
    {
        mDataPrivate->getDownloadResponse().setErrorData(convertToServiceErrorStruct(*errorData));
    }
}

void NetworkHttpDownloadToFileHandler::appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished)
{
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

void NetworkHttpDownloadToFileHandler::completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics)
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