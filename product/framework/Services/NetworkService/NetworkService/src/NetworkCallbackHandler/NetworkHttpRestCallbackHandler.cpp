#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>
#include <ucf/Services/NetworkService/Model/HttpRestRequest.h>
#include <ucf/Services/NetworkService/Model/HttpRestResponse.h>


#include "NetworkHttpRestCallbackHandler.h"

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpRestCallbackHandler::DataPrivate{
public:
    DataPrivate(const ucf::service::network::http::HttpRestRequest& restRequest, const HttpRestResponseCallbackFunc& restResponseCallback);
    
    const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::utilities::network::http::NetworkHttpResponse& getHttpResponse(){return mHttpResponse;}
    ucf::service::network::http::HttpRestResponseCallbackFunc getResponseCallback() const{return mResponseCallBack;}

    void appendBuffer(const ucf::utilities::network::http::ByteBuffer& buffer){mCachedBuffer.insert(mCachedBuffer.end(), buffer.begin(), buffer.end());}
    const ucf::utilities::network::http::ByteBuffer& getCachedBuffer() const{return mCachedBuffer;}

    int getRetryCount() const{ return mRetryCount;}
    void parepareRetry();

    void convertRestRequestToHttpRequest(const ucf::service::network::http::HttpRestRequest& restRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const;
    void convertHttpResponseToRestResponse(const ucf::utilities::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRestResponse& restResponse) const;
private:
    ucf::service::network::http::HttpRestResponseCallbackFunc mResponseCallBack;
    ucf::utilities::network::http::NetworkHttpResponse mHttpResponse;
    ucf::utilities::network::http::NetworkHttpRequest mHttpRequest;
    ucf::utilities::network::http::ByteBuffer mCachedBuffer;
    int mRetryCount;
};

NetworkHttpRestCallbackHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpRestRequest& restRequest, const ucf::service::network::http::HttpRestResponseCallbackFunc& restResponseCallback)
    : mResponseCallBack(restResponseCallback)
    , mRetryCount(0)
{
    convertRestRequestToHttpRequest(restRequest, mHttpRequest);
}

void NetworkHttpRestCallbackHandler::DataPrivate::parepareRetry()
{
    ++mRetryCount;
}

void NetworkHttpRestCallbackHandler::DataPrivate::convertRestRequestToHttpRequest(const ucf::service::network::http::HttpRestRequest& restRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const
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
    httpRequest.setPayloadJsonString(restRequest.getPayloadJsonString());
}

void NetworkHttpRestCallbackHandler::DataPrivate::convertHttpResponseToRestResponse(const ucf::utilities::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRestResponse& restResponse) const
{

}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpRestCallbackHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpRestCallbackHandler::NetworkHttpRestCallbackHandler(const ucf::service::network::http::HttpRestRequest& restRequest, const HttpRestResponseCallbackFunc& restResponseCallback)
    : mDataPrivate(std::make_unique<NetworkHttpRestCallbackHandler::DataPrivate>(restRequest, restResponseCallback))
{

}

NetworkHttpRestCallbackHandler::~NetworkHttpRestCallbackHandler()
{

}

const ucf::utilities::network::http::NetworkHttpRequest& NetworkHttpRestCallbackHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpRestCallbackHandler::setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData)
{
    mDataPrivate->getHttpResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getHttpResponse().setResponseHeaders(headers);
    if (errorData)
    {
        mDataPrivate->getHttpResponse().setErrorData(*errorData);
    }
}

void NetworkHttpRestCallbackHandler::appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished)
{
    mDataPrivate->appendBuffer(buffer);
    if (isFinished)
    {
        std::string bodyString = std::string(mDataPrivate->getCachedBuffer().begin(), mDataPrivate->getCachedBuffer().end());
        mDataPrivate->getHttpResponse().setResponseBody(std::move(bodyString));
    }
}

void NetworkHttpRestCallbackHandler::completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics)
{
    if (mDataPrivate->getResponseCallback())
    {
        ucf::service::network::http::HttpRestResponse restResponse;
        mDataPrivate->convertHttpResponseToRestResponse(mDataPrivate->getHttpResponse(), restResponse);
        mDataPrivate->getResponseCallback()(restResponse);
    }
}

bool NetworkHttpRestCallbackHandler::shouldRetryRequest() const
{
    if (mDataPrivate->getHttpResponse().getHttpResponseCode() == 302)
    {
        return mDataPrivate->getRetryCount() <= 2;
    }
    return false;
}

const ucf::utilities::network::http::NetworkHttpRequest& NetworkHttpRestCallbackHandler::prepareRetryRequest()
{
    mDataPrivate->parepareRetry();
    return getHttpRequest();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpRestCallbackHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}