
#include "NetworkHttpDownloadToMemoryHandler.h"

#include <ucf/infrastructure/NetworkClient/LibCurlClient/HttpRedirect.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpTypes.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpRequest.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpResponse.h>
#include <ucf/services/NetworkService/model/HttpDownloadToMemoryRequest.h>
#include <ucf/services/NetworkService/model/HttpDownloadToMemoryResponse.h>

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
    
    const ucf::infrastructure::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::service::network::http::HttpDownloadToMemoryResponse& getDownloadResponse(){return mDownloadResponse;}
    const ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc& getResponseCallback() const{return mResponseCallBack;}

    bool tryPrepareRedirectRequest();

    void convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const;
private:
    ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc mResponseCallBack;
    ucf::service::network::http::HttpDownloadToMemoryResponse mDownloadResponse;
    ucf::infrastructure::network::http::NetworkHttpRequest mHttpRequest;
    int mRedirectCount{0};
};

NetworkHttpDownloadToMemoryHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc& downloadResponseCallback)
    : mResponseCallBack(downloadResponseCallback)
{
    convertDownloadRequestToHttpRequest(downloadRequest, mHttpRequest);
}

void NetworkHttpDownloadToMemoryHandler::DataPrivate::convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const
{
    httpRequest.setRequestMethod(ucf::infrastructure::network::http::HTTPMethod::GET);
    httpRequest.setRequestHeaders(downloadRequest.getRequestHeaders());
    httpRequest.setRequestId(downloadRequest.getRequestId());
    httpRequest.setTrackingId(downloadRequest.getTrackingId());
    httpRequest.setRequestUri(downloadRequest.getRequestUri());
    httpRequest.setTimeout(downloadRequest.getTimeout());
}

bool NetworkHttpDownloadToMemoryHandler::DataPrivate::tryPrepareRedirectRequest()
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

const ucf::infrastructure::network::http::NetworkHttpRequest& NetworkHttpDownloadToMemoryHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpDownloadToMemoryHandler::setResponseHeader(int statusCode, const ucf::infrastructure::network::http::NetworkHttpHeaders& headers, std::optional<ucf::infrastructure::network::http::ResponseErrorStruct> errorData)
{

    mDataPrivate->getDownloadResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getDownloadResponse().setResponseHeaders(headers);
    
    if (errorData)
    {
        mDataPrivate->getDownloadResponse().setErrorData(convertToServiceErrorStruct(*errorData));
    }
}

void NetworkHttpDownloadToMemoryHandler::appendResponseBody(const ucf::infrastructure::network::http::ByteBuffer& buffer, bool isFinished)
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

void NetworkHttpDownloadToMemoryHandler::completeResponse(const ucf::infrastructure::network::http::HttpResponseMetrics& metrics)
{
    if (auto callback = mDataPrivate->getResponseCallback())
    {
        mDataPrivate->getDownloadResponse().setFinished();
        callback(mDataPrivate->getDownloadResponse());
    }
}

bool NetworkHttpDownloadToMemoryHandler::tryPrepareRedirectRequest()
{
    return mDataPrivate->tryPrepareRedirectRequest();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpDownloadToMemoryHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}