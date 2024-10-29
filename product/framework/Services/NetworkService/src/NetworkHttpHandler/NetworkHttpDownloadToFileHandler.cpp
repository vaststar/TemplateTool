
#include "NetworkHttpDownloadToFileHandler.h"

#include <fstream>

#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToFileRequest.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToFileResponse.h>

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
    ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc getResponseCallback() const{return mResponseCallBack;}

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
};

NetworkHttpDownloadToFileHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpDownloadToFileRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToFileResponseCallbackFunc& restResponseCallback)
    : mResponseCallBack(restResponseCallback)
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
        mOutFilestream.flush();
        return true;
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
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpDownloadToFileHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}