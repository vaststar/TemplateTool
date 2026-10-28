
#include "NetworkHttpRestHandler.h"

#include <ucf/infrastructure/NetworkClient/LibCurlClient/HttpRedirect.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpTypes.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpRequest.h>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpResponse.h>
#include <ucf/services/NetworkService/model/HttpRestRequest.h>
#include <ucf/services/NetworkService/model/HttpRestResponse.h>

#include "NetworkHttpTypeConverter.h"

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpRestHandler::DataPrivate{
public:
    DataPrivate(const ucf::service::network::http::HttpRestRequest& restRequest, const HttpRestResponseCallbackFunc& restResponseCallback);
    
    const ucf::infrastructure::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::infrastructure::network::http::NetworkHttpResponse& getHttpResponse(){return mHttpResponse;}
    const ucf::service::network::http::HttpRestResponseCallbackFunc& getResponseCallback() const{return mResponseCallBack;}

    bool tryPrepareRedirectRequest();

    void prepareRetry();

    void convertRestRequestToHttpRequest(const ucf::service::network::http::HttpRestRequest& restRequest, ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const;
    void convertHttpResponseToRestResponse(const ucf::infrastructure::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRestResponse& restResponse) const;
private:
    ucf::service::network::http::HttpRestResponseCallbackFunc mResponseCallBack;
    ucf::infrastructure::network::http::NetworkHttpResponse mHttpResponse;
    ucf::infrastructure::network::http::NetworkHttpRequest mHttpRequest;
    int mRedirectCount;
};

NetworkHttpRestHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpRestRequest& restRequest, const ucf::service::network::http::HttpRestResponseCallbackFunc& restResponseCallback)
    : mResponseCallBack(restResponseCallback)
    , mRedirectCount(0)
{
    convertRestRequestToHttpRequest(restRequest, mHttpRequest);
}

bool NetworkHttpRestHandler::DataPrivate::tryPrepareRedirectRequest()
{
    if (mHttpResponse.getErrorData().has_value() || mRedirectCount >= kMaxRedirectCount)
    {
        return false;
    }
    if (!ucf::infrastructure::network::libcurl::tryApplyHttpRedirect(
            mHttpRequest, mHttpResponse.getHttpResponseCode(),
            mHttpResponse.getResponseHeaders()))
    {
        return false;
    }

    ++mRedirectCount;
    mHttpResponse.clear();
    return true;
}

void NetworkHttpRestHandler::DataPrivate::prepareRetry()
{
    mHttpResponse.clear();
}

void NetworkHttpRestHandler::DataPrivate::convertRestRequestToHttpRequest(const ucf::service::network::http::HttpRestRequest& restRequest, ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const
{
    httpRequest.setRequestMethod(convertToUtilitiesHttpMethod(restRequest.getRequestMethod()));
    httpRequest.setRequestHeaders(restRequest.getRequestHeaders());
    httpRequest.setRequestId(restRequest.getRequestId());
    httpRequest.setTrackingId(restRequest.getTrackingId());
    httpRequest.setRequestUri(restRequest.getRequestUri());
    httpRequest.setTimeout(restRequest.getTimeout());
    httpRequest.setPayloadString(restRequest.getPayloadJsonString());
}

void NetworkHttpRestHandler::DataPrivate::convertHttpResponseToRestResponse(const ucf::infrastructure::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRestResponse& restResponse) const
{
    restResponse.setHttpResponseCode(httpResponse.getHttpResponseCode());
    restResponse.setResponseHeaders(httpResponse.getResponseHeaders());
    if (const auto& responseBody = httpResponse.getResponseBody(); !responseBody.empty())
    {
        restResponse.setResponseBody(std::string{responseBody.begin(), responseBody.end()});
    }
    
    if (auto errorData = httpResponse.getErrorData())
    {
        restResponse.setErrorData(convertToServiceErrorStruct(*errorData));
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpRestHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpRestHandler::NetworkHttpRestHandler(const ucf::service::network::http::HttpRestRequest& restRequest, const HttpRestResponseCallbackFunc& restResponseCallback)
    : mDataPrivate(std::make_unique<NetworkHttpRestHandler::DataPrivate>(restRequest, restResponseCallback))
{

}

NetworkHttpRestHandler::~NetworkHttpRestHandler()
{

}

const ucf::infrastructure::network::http::NetworkHttpRequest& NetworkHttpRestHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpRestHandler::setResponseHeader(int statusCode, const ucf::infrastructure::network::http::NetworkHttpHeaders& headers, std::optional<ucf::infrastructure::network::http::ResponseErrorStruct> errorData)
{
    mDataPrivate->getHttpResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getHttpResponse().setResponseHeaders(headers);
    if (errorData)
    {
        mDataPrivate->getHttpResponse().setErrorData(*errorData);
    }
}

void NetworkHttpRestHandler::appendResponseBody(const ucf::infrastructure::network::http::ByteBuffer& buffer, bool isFinished)
{
    if (!buffer.empty())
    {
        mDataPrivate->getHttpResponse().appendResponseBody(buffer);
    }
}

void NetworkHttpRestHandler::completeResponse(const ucf::infrastructure::network::http::HttpResponseMetrics& metrics)
{
    if (mDataPrivate->getResponseCallback())
    {
        ucf::service::network::http::HttpRestResponse restResponse;
        mDataPrivate->convertHttpResponseToRestResponse(mDataPrivate->getHttpResponse(), restResponse);
        mDataPrivate->getResponseCallback()(restResponse);
    }
}

bool NetworkHttpRestHandler::tryPrepareRedirectRequest()
{
    return mDataPrivate->tryPrepareRedirectRequest();
}

bool NetworkHttpRestHandler::shouldRetryRequest() const
{
    return false;
}

int NetworkHttpRestHandler::getRetryAfterMillSecs() const
{
    return 0;
}

void NetworkHttpRestHandler::prepareRetryRequest()
{
    if (shouldRetryRequest())
    {
        mDataPrivate->prepareRetry();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpRestHandler Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
