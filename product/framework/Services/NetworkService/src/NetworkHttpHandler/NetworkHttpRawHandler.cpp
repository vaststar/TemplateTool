
#include "NetworkHttpRawHandler.h"

#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>
#include <ucf/Services/NetworkService/Model/HttpRawRequest.h>
#include <ucf/Services/NetworkService/Model/HttpRawResponse.h>

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpRawHandler::DataPrivate{
public:
    DataPrivate(const ucf::service::network::http::HttpRawRequest& restRequest, const HttpRawResponseCallbackFunc& rawResponseCallback);
    
    const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::utilities::network::http::NetworkHttpResponse& getHttpResponse(){return mHttpResponse;}
    ucf::service::network::http::HttpRawResponseCallbackFunc getResponseCallback() const{return mResponseCallBack;}

    void appendBuffer(const ucf::utilities::network::http::ByteBuffer& buffer){mCachedBuffer.insert(mCachedBuffer.end(), buffer.begin(), buffer.end());}
    const ucf::utilities::network::http::ByteBuffer& getCachedBuffer() const{return mCachedBuffer;}

    void convertRawRequestToHttpRequest(const ucf::service::network::http::HttpRawRequest& rawRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const;
    void convertHttpResponseToRestResponse(const ucf::utilities::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRawResponse& rawResponse) const;
private:
    ucf::service::network::http::HttpRawResponseCallbackFunc mResponseCallBack;
    ucf::utilities::network::http::NetworkHttpResponse mHttpResponse;
    ucf::utilities::network::http::NetworkHttpRequest mHttpRequest;
    ucf::utilities::network::http::ByteBuffer mCachedBuffer;
    int mRedirectCount;
};

NetworkHttpRawHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpRawRequest& rawRequest, const ucf::service::network::http::HttpRawResponseCallbackFunc& rawResponseCallback)
    : mResponseCallBack(rawResponseCallback)
    , mRedirectCount(0)
{
    convertRawRequestToHttpRequest(rawRequest, mHttpRequest);
}

void NetworkHttpRawHandler::DataPrivate::convertRawRequestToHttpRequest(const ucf::service::network::http::HttpRawRequest& restRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const
{
    ucf::utilities::network::http::HTTPMethod httpMethod = ucf::utilities::network::http::HTTPMethod::GET;
    switch (restRequest.getRequestMethod())
    {
    case HTTPMethod::GET:
        httpMethod = ucf::utilities::network::http::HTTPMethod::GET;
        break;
    case HTTPMethod::POST:
        httpMethod = ucf::utilities::network::http::HTTPMethod::POST;
        break;
    case HTTPMethod::HEAD:
        httpMethod = ucf::utilities::network::http::HTTPMethod::HEAD;
        break;
    case HTTPMethod::PUT:
        httpMethod = ucf::utilities::network::http::HTTPMethod::PUT;
        break;
    case HTTPMethod::DEL:
        httpMethod = ucf::utilities::network::http::HTTPMethod::DEL;
        break;
    case HTTPMethod::PATCH:
        httpMethod = ucf::utilities::network::http::HTTPMethod::PATCH;
        break;
    case HTTPMethod::OPTIONS:
        httpMethod = ucf::utilities::network::http::HTTPMethod::OPTIONS;
        break;
    default:
        break;
    }
    httpRequest.setRequestMethod(httpMethod);
    httpRequest.setRequestHeaders(restRequest.getRequestHeaders());
    httpRequest.setRequestId(restRequest.getRequestId());
    httpRequest.setTrackingId(restRequest.getTrackingId());
    httpRequest.setRequestUri(restRequest.getRequestUri());
    httpRequest.setTimeout(restRequest.getTimeout());
    httpRequest.setPayloadString(restRequest.getPayloadString());
}

void NetworkHttpRawHandler::DataPrivate::convertHttpResponseToRestResponse(const ucf::utilities::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRawResponse& rawResponse) const
{
    rawResponse.setHttpResponseCode(httpResponse.getHttpResponseCode());
    rawResponse.setResponseHeaders(httpResponse.getResponseHeaders());
    if (const auto& responseBody = httpResponse.getResponseBody(); !responseBody.empty())
    {
        rawResponse.setResponseBody(responseBody);
    }
    
    if (auto errorData = httpResponse.getErrorData())
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
        rawResponse.setErrorData(restErrorData);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpRawHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpRawHandler::NetworkHttpRawHandler(const ucf::service::network::http::HttpRawRequest& rawRequest, const HttpRawResponseCallbackFunc& rawResponseCallback)
    : mDataPrivate(std::make_unique<NetworkHttpRawHandler::DataPrivate>(rawRequest, rawResponseCallback))
{

}

NetworkHttpRawHandler::~NetworkHttpRawHandler()
{

}

const ucf::utilities::network::http::NetworkHttpRequest& NetworkHttpRawHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpRawHandler::setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData)
{
    mDataPrivate->getHttpResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getHttpResponse().setResponseHeaders(headers);
    if (errorData)
    {
        mDataPrivate->getHttpResponse().setErrorData(*errorData);
    }
}

void NetworkHttpRawHandler::appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished)
{
    if (!buffer.empty())
    {
        mDataPrivate->getHttpResponse().appendResponseBody(buffer);
    }
}

void NetworkHttpRawHandler::completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics)
{
    if (mDataPrivate->getResponseCallback())
    {
        ucf::service::network::http::HttpRawResponse rawResponse;
        mDataPrivate->convertHttpResponseToRestResponse(mDataPrivate->getHttpResponse(), rawResponse);
        mDataPrivate->getResponseCallback()(rawResponse);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpRawHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}