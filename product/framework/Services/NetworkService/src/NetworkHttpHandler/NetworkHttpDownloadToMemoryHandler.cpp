
#include "NetworkHttpDownloadToMemoryHandler.h"

#include <algorithm>
#include <cctype>

#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToMemoryRequest.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToMemoryResponse.h>

#include "NetworkHttpTypeConverter.h"


namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpDownloadToMemoryHandler::DataPrivate{
public:
    DataPrivate(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, const HttpDownloadToMemoryResponseCallbackFunc& downloadResponseCallback);
    
    const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::service::network::http::HttpDownloadToMemoryResponse& getDownloadResponse(){return mDownloadResponse;}
    const ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc& getResponseCallback() const{return mResponseCallBack;}

    int getRedirectCount() const{ return mRedirectCount;}
    void prepareRedirect();

    void convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const;
private:
    ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc mResponseCallBack;
    ucf::service::network::http::HttpDownloadToMemoryResponse mDownloadResponse;
    ucf::utilities::network::http::NetworkHttpRequest mHttpRequest;
    int mRedirectCount{0};
};

NetworkHttpDownloadToMemoryHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc& downloadResponseCallback)
    : mResponseCallBack(downloadResponseCallback)
{
    convertDownloadRequestToHttpRequest(downloadRequest, mHttpRequest);
}

void NetworkHttpDownloadToMemoryHandler::DataPrivate::convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const
{
    httpRequest.setRequestMethod(ucf::utilities::network::http::HTTPMethod::GET);
    httpRequest.setRequestHeaders(downloadRequest.getRequestHeaders());
    httpRequest.setRequestId(downloadRequest.getRequestId());
    httpRequest.setTrackingId(downloadRequest.getTrackingId());
    httpRequest.setRequestUri(downloadRequest.getRequestUri());
    httpRequest.setTimeout(downloadRequest.getTimeout());
}

void NetworkHttpDownloadToMemoryHandler::DataPrivate::prepareRedirect()
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
////////////////////Start NetworkHttpDownloadToMemoryHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpDownloadToMemoryHandler::NetworkHttpDownloadToMemoryHandler(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, const HttpDownloadToMemoryResponseCallbackFunc& restResponseCallback)
    : mDataPrivate(std::make_unique<NetworkHttpDownloadToMemoryHandler::DataPrivate>(downloadRequest, restResponseCallback))
{

}

NetworkHttpDownloadToMemoryHandler::~NetworkHttpDownloadToMemoryHandler()
{

}

const ucf::utilities::network::http::NetworkHttpRequest& NetworkHttpDownloadToMemoryHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpDownloadToMemoryHandler::setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData)
{

    mDataPrivate->getDownloadResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getDownloadResponse().setResponseHeaders(headers);
    
    if (errorData)
    {
        mDataPrivate->getDownloadResponse().setErrorData(convertToServiceErrorStruct(*errorData));
    }
}

void NetworkHttpDownloadToMemoryHandler::appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished)
{
    if (!buffer.empty())
    {
        mDataPrivate->getDownloadResponse().appendResponseBody(buffer);
        
        if (auto callback = mDataPrivate->getResponseCallback())
        {
            callback(mDataPrivate->getDownloadResponse());
        }
    }
}

void NetworkHttpDownloadToMemoryHandler::completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics)
{
    if (auto callback = mDataPrivate->getResponseCallback())
    {
        mDataPrivate->getDownloadResponse().setFinished();
        callback(mDataPrivate->getDownloadResponse());
    }
}

bool NetworkHttpDownloadToMemoryHandler::shouldRedirectRequest() const
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

void NetworkHttpDownloadToMemoryHandler::prepareRedirectRequest()
{
    if (shouldRedirectRequest())
    {
        mDataPrivate->prepareRedirect();
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpDownloadToMemoryHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}