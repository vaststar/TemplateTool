
#include "NetworkHttpRestHandler.h"

#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>
#include <ucf/Services/NetworkService/Model/HttpRestRequest.h>
#include <ucf/Services/NetworkService/Model/HttpRestResponse.h>

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
    
    const ucf::utilities::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::utilities::network::http::NetworkHttpResponse& getHttpResponse(){return mHttpResponse;}
    const ucf::service::network::http::HttpRestResponseCallbackFunc& getResponseCallback() const{return mResponseCallBack;}

    int getRedirectCount() const{ return mRedirectCount;}
    void prepareRedirect();

    void prepareRetry();

    void convertRestRequestToHttpRequest(const ucf::service::network::http::HttpRestRequest& restRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const;
    void convertHttpResponseToRestResponse(const ucf::utilities::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRestResponse& restResponse) const;
private:
    ucf::service::network::http::HttpRestResponseCallbackFunc mResponseCallBack;
    ucf::utilities::network::http::NetworkHttpResponse mHttpResponse;
    ucf::utilities::network::http::NetworkHttpRequest mHttpRequest;
    int mRedirectCount;
};

NetworkHttpRestHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpRestRequest& restRequest, const ucf::service::network::http::HttpRestResponseCallbackFunc& restResponseCallback)
    : mResponseCallBack(restResponseCallback)
    , mRedirectCount(0)
{
    convertRestRequestToHttpRequest(restRequest, mHttpRequest);
}

void NetworkHttpRestHandler::DataPrivate::prepareRedirect()
{
    ++mRedirectCount;
    if (auto redirectUri = mHttpResponse.getHeaderValue("Location"))
    {
        mHttpRequest.setRequestUri(*redirectUri);
    }
    
    mHttpResponse.clear();
}

void NetworkHttpRestHandler::DataPrivate::prepareRetry()
{
    mHttpResponse.clear();
}

void NetworkHttpRestHandler::DataPrivate::convertRestRequestToHttpRequest(const ucf::service::network::http::HttpRestRequest& restRequest, ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const
{
    httpRequest.setRequestMethod(convertToUtilitiesHttpMethod(restRequest.getRequestMethod()));
    httpRequest.setRequestHeaders(restRequest.getRequestHeaders());
    httpRequest.setRequestId(restRequest.getRequestId());
    httpRequest.setTrackingId(restRequest.getTrackingId());
    httpRequest.setRequestUri(restRequest.getRequestUri());
    httpRequest.setTimeout(restRequest.getTimeout());
    httpRequest.setPayloadString(restRequest.getPayloadJsonString());
}

void NetworkHttpRestHandler::DataPrivate::convertHttpResponseToRestResponse(const ucf::utilities::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRestResponse& restResponse) const
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

const ucf::utilities::network::http::NetworkHttpRequest& NetworkHttpRestHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpRestHandler::setResponseHeader(int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData)
{
    mDataPrivate->getHttpResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getHttpResponse().setResponseHeaders(headers);
    if (errorData)
    {
        mDataPrivate->getHttpResponse().setErrorData(*errorData);
    }
}

void NetworkHttpRestHandler::appendResponseBody(const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished)
{
    if (!buffer.empty())
    {
        mDataPrivate->getHttpResponse().appendResponseBody(buffer);
    }
}

void NetworkHttpRestHandler::completeResponse(const ucf::utilities::network::http::HttpResponseMetrics& metrics)
{
    if (mDataPrivate->getResponseCallback())
    {
        ucf::service::network::http::HttpRestResponse restResponse;
        mDataPrivate->convertHttpResponseToRestResponse(mDataPrivate->getHttpResponse(), restResponse);
        mDataPrivate->getResponseCallback()(restResponse);
    }
}

bool NetworkHttpRestHandler::shouldRedirectRequest() const
{
    if (301 == mDataPrivate->getHttpResponse().getHttpResponseCode() ||
        302 == mDataPrivate->getHttpResponse().getHttpResponseCode() ||
        303 == mDataPrivate->getHttpResponse().getHttpResponseCode() ||
        307 == mDataPrivate->getHttpResponse().getHttpResponseCode() ||
        308 == mDataPrivate->getHttpResponse().getHttpResponseCode())
    {
        return mDataPrivate->getRedirectCount() < kMaxRedirectCount;
    }
    return false;
}

void NetworkHttpRestHandler::prepareRedirectRequest()
{
    if (shouldRedirectRequest())
    {
        mDataPrivate->prepareRedirect();
    }
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