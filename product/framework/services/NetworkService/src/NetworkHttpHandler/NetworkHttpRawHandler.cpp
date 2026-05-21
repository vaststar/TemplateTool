
#include "NetworkHttpRawHandler.h"

#include <ucf/Infrastructure/NetworkClient/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Infrastructure/NetworkClient/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Infrastructure/NetworkClient/NetworkModelTypes/Http/NetworkHttpResponse.h>
#include <ucf/Services/NetworkService/Model/HttpRawRequest.h>
#include <ucf/Services/NetworkService/Model/HttpRawResponse.h>

#include "NetworkHttpTypeConverter.h"

namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpRawHandler::DataPrivate{
public:
    DataPrivate(const ucf::service::network::http::HttpRawRequest& restRequest, const HttpRawResponseCallbackFunc& rawResponseCallback);
    
    const ucf::infrastructure::network::http::NetworkHttpRequest& getHttpRequest() const{ return mHttpRequest;}
    ucf::infrastructure::network::http::NetworkHttpResponse& getHttpResponse(){return mHttpResponse;}
    const ucf::service::network::http::HttpRawResponseCallbackFunc& getResponseCallback() const{return mResponseCallBack;}

    void convertRawRequestToHttpRequest(const ucf::service::network::http::HttpRawRequest& rawRequest, ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const;
    void convertHttpResponseToRestResponse(const ucf::infrastructure::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRawResponse& rawResponse) const;
private:
    ucf::service::network::http::HttpRawResponseCallbackFunc mResponseCallBack;
    ucf::infrastructure::network::http::NetworkHttpResponse mHttpResponse;
    ucf::infrastructure::network::http::NetworkHttpRequest mHttpRequest;
};

NetworkHttpRawHandler::DataPrivate::DataPrivate(const ucf::service::network::http::HttpRawRequest& rawRequest, const ucf::service::network::http::HttpRawResponseCallbackFunc& rawResponseCallback)
    : mResponseCallBack(rawResponseCallback)
{
    convertRawRequestToHttpRequest(rawRequest, mHttpRequest);
}

void NetworkHttpRawHandler::DataPrivate::convertRawRequestToHttpRequest(const ucf::service::network::http::HttpRawRequest& restRequest, ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const
{
    httpRequest.setRequestMethod(convertToUtilitiesHttpMethod(restRequest.getRequestMethod()));
    httpRequest.setRequestHeaders(restRequest.getRequestHeaders());
    httpRequest.setRequestId(restRequest.getRequestId());
    httpRequest.setTrackingId(restRequest.getTrackingId());
    httpRequest.setRequestUri(restRequest.getRequestUri());
    httpRequest.setTimeout(restRequest.getTimeout());
    httpRequest.setPayloadString(restRequest.getPayloadString());
}

void NetworkHttpRawHandler::DataPrivate::convertHttpResponseToRestResponse(const ucf::infrastructure::network::http::NetworkHttpResponse& httpResponse, ucf::service::network::http::HttpRawResponse& rawResponse) const
{
    rawResponse.setHttpResponseCode(httpResponse.getHttpResponseCode());
    rawResponse.setResponseHeaders(httpResponse.getResponseHeaders());
    if (const auto& responseBody = httpResponse.getResponseBody(); !responseBody.empty())
    {
        rawResponse.setResponseBody(responseBody);
    }
    
    if (auto errorData = httpResponse.getErrorData())
    {
        rawResponse.setErrorData(convertToServiceErrorStruct(*errorData));
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

const ucf::infrastructure::network::http::NetworkHttpRequest& NetworkHttpRawHandler::getHttpRequest() const
{
    return mDataPrivate->getHttpRequest();
}

void NetworkHttpRawHandler::setResponseHeader(int statusCode, const ucf::infrastructure::network::http::NetworkHttpHeaders& headers, std::optional<ucf::infrastructure::network::http::ResponseErrorStruct> errorData)
{
    mDataPrivate->getHttpResponse().setHttpResponseCode(statusCode);
    mDataPrivate->getHttpResponse().setResponseHeaders(headers);
    if (errorData)
    {
        mDataPrivate->getHttpResponse().setErrorData(*errorData);
    }
}

void NetworkHttpRawHandler::appendResponseBody(const ucf::infrastructure::network::http::ByteBuffer& buffer, bool isFinished)
{
    if (!buffer.empty())
    {
        mDataPrivate->getHttpResponse().appendResponseBody(buffer);
    }
}

void NetworkHttpRawHandler::completeResponse(const ucf::infrastructure::network::http::HttpResponseMetrics& metrics)
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