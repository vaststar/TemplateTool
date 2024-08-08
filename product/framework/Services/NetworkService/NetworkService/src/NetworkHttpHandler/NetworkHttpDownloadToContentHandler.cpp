#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToContentRequest.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToContentResponse.h>


#include "NetworkHttpDownloadToContentHandler.h"

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpDownloadToContentHandler::DataPrivate{
public:
    DataPrivate(const ucf::service::network::http::HttpDownloadToContentRequest& downloadRequest, const HttpDownloadToContentResponseCallbackFunc& restResponseCallback);
    
    const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::service::network::http::HttpDownloadToContentResponse& getDownloadResponse(){return mDownloadResponse;}
    ucf::service::network::http::HttpDownloadToContentResponseCallbackFunc getResponseCallback() const{return mResponseCallBack;}

    void convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToContentRequest& downloadRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const;
private:
    ucf::service::network::http::HttpDownloadToContentResponseCallbackFunc mResponseCallBack;
    ucf::service::network::http::HttpDownloadToContentResponse mDownloadResponse;
    ucf::utilities::network::http::NetworkHttpRequest mHttpRequest;
};

NetworkHttpDownloadToContentHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpDownloadToContentRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToContentResponseCallbackFunc& restResponseCallback)
    : mResponseCallBack(restResponseCallback)
{
    convertDownloadRequestToHttpRequest(downloadRequest, mHttpRequest);
}

void NetworkHttpDownloadToContentHandler::DataPrivate::convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToContentRequest& downloadRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const
{
    httpRequest.setRequestMethod(ucf::utilities::network::http::HTTPMethod::GET);
    httpRequest.setRequestHeaders(downloadRequest.getRequestHeaders());
    httpRequest.setRequestId(downloadRequest.getRequestId());
    httpRequest.setTrackingId(downloadRequest.getTrackingId());
    httpRequest.setRequestUri(downloadRequest.getRequestUri());
    httpRequest.setTimeout(downloadRequest.getTimeout());
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpDownloadToContentHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpDownloadToContentHandler::NetworkHttpDownloadToContentHandler(const ucf::service::network::http::HttpDownloadToContentRequest& downloadRequest, const HttpDownloadToContentResponseCallbackFunc& restResponseCallback)
    : mDataPrivate(std::make_unique<NetworkHttpDownloadToContentHandler::DataPrivate>(downloadRequest, restResponseCallback))
{

}

NetworkHttpDownloadToContentHandler::~NetworkHttpDownloadToContentHandler()
{

}

const ucf::utilities::network::http::NetworkHttpRequest& NetworkHttpDownloadToContentHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpDownloadToContentHandler::setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData)
{

    mDataPrivate->getDownloadResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getDownloadResponse().setResponseHeaders(headers);
    
    if (errorData)
    {
        ResponseErrorStruct restErrorData;
        restErrorData.errorCode = errorData->errorCode;
        restErrorData.errorDescription = errorData->errorDescription;
        switch (errorData->errorType)
        {
        case ucf::utilities::network::http::ResponseErrorType::NoError:
            restErrorData.errorType = ucf::service::network::http::ResponseErrorType::NoError;
            break;
        case ucf::utilities::network::http::ResponseErrorType::DNSError:
            restErrorData.errorType = ucf::service::network::http::ResponseErrorType::DNSError;
            break;
        case ucf::utilities::network::http::ResponseErrorType::SocketError:
            restErrorData.errorType = ucf::service::network::http::ResponseErrorType::SocketError;
            break;
        case ucf::utilities::network::http::ResponseErrorType::TLSError:
            restErrorData.errorType = ucf::service::network::http::ResponseErrorType::TLSError;
            break;
        case ucf::utilities::network::http::ResponseErrorType::TimeoutError:
            restErrorData.errorType = ucf::service::network::http::ResponseErrorType::TimeoutError;
            break;
        case ucf::utilities::network::http::ResponseErrorType::CanceledError:
            restErrorData.errorType = ucf::service::network::http::ResponseErrorType::CanceledError;
            break;
        case ucf::utilities::network::http::ResponseErrorType::OtherError:
            restErrorData.errorType = ucf::service::network::http::ResponseErrorType::OtherError;
            break;
        case ucf::utilities::network::http::ResponseErrorType::UnHandledError:
            restErrorData.errorType = ucf::service::network::http::ResponseErrorType::UnHandledError;
            break;
        default:
            break;
        }
        mDataPrivate->getDownloadResponse().setErrorData(restErrorData);
    }
}

void NetworkHttpDownloadToContentHandler::appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished)
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

void NetworkHttpDownloadToContentHandler::completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics)
{
    if (auto callback = mDataPrivate->getResponseCallback())
    {
        callback(mDataPrivate->getDownloadResponse());
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpDownloadToContentHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}