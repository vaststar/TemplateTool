
#include "NetworkHttpDownloadToMemoryHandler.h"

#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToMemoryRequest.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToMemoryResponse.h>


namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpDownloadToMemoryHandler::DataPrivate{
public:
    DataPrivate(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, const HttpDownloadToMemoryResponseCallbackFunc& restResponseCallback);
    
    const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::service::network::http::HttpDownloadToMemoryResponse& getDownloadResponse(){return mDownloadResponse;}
    ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc getResponseCallback() const{return mResponseCallBack;}

    void convertDownloadRequestToHttpRequest(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const;
private:
    ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc mResponseCallBack;
    ucf::service::network::http::HttpDownloadToMemoryResponse mDownloadResponse;
    std::string mDownloadFilePath;
    ucf::utilities::network::http::NetworkHttpRequest mHttpRequest;
};

NetworkHttpDownloadToMemoryHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpDownloadToMemoryRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToMemoryResponseCallbackFunc& restResponseCallback)
    : mResponseCallBack(restResponseCallback)
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
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpDownloadToMemoryHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}