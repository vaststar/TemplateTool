#include "NetworkHttpManager.h"
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>
#include <ucf/Utilities/NetworkUtils/LibCurlClient/LibCurlClient.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>

#include "NetworkCallbackHandler/NetworkHttpRestCallbackHandler.h"

#ifndef RETURN_FROM_LAMBDA_IF_DEAD
#define RETURN_FROM_LAMBDA_IF_DEAD(x) \
    auto sharedThis = x.lock();       \
    if (!sharedThis)                  \
        return;
#endif
namespace ucf::service::network::http{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkHttpManager::DataPrivate
{
public:
    DataPrivate();
    std::shared_ptr<ucf::utilities::network::libcurl::LibCurlClient> getLibCurlClient() const;
private:
    std::shared_ptr<ucf::utilities::network::libcurl::LibCurlClient> mLibcurlClient;
};

NetworkHttpManager::DataPrivate::DataPrivate()
    :mLibcurlClient(std::make_shared<ucf::utilities::network::libcurl::LibCurlClient>())
{

}

std::shared_ptr<ucf::utilities::network::libcurl::LibCurlClient> NetworkHttpManager::DataPrivate::getLibCurlClient() const
{
    return mLibcurlClient;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkHttpManager Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
NetworkHttpManager::NetworkHttpManager()
    : mDataPrivate(std::make_unique<DataPrivate>())
{
    SERVICE_LOG_DEBUG("Create NetworkHttpManager, address:" << this);
}
 
NetworkHttpManager::~NetworkHttpManager()
{
   SERVICE_LOG_DEBUG("");
}

void NetworkHttpManager::sendHttpRestRequest(const ucf::service::network::http::HttpRestRequest& restRequest, const HttpRestResponseCallbackFunc& restResponseCallback, const std::source_location location)
{
    auto callbackHandler = std::make_shared<NetworkHttpRestCallbackHandler>(restRequest, restResponseCallback);
    sendHttpRequest(callbackHandler->getHttpRequest(), callbackHandler, location);
}

void NetworkHttpManager::sendHttpRequest(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest, 
                         std::shared_ptr<INetworkHttpCallbackHandler> callbackHandler,
                         const std::source_location location)
{
   SERVICE_LOG_DEBUG("about making http request:" << httpRequest.toString() <<", from: " 
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());

//     using json = nlohmann::json;
//     // parse explicitly
//     auto j = json::parse(R"({"happy": true, "pi": 3.141})");
//     // explicit conversion to string
// std::string s = j.dump();    // {"happy":true,"pi":3.141}
//     SERVICE_LOG_DEBUG("test:" << s);

    std::string trackingId = httpRequest.getTrackingId();
    std::string requestId = httpRequest.getRequestId();
    auto curlHeaderCallback = [callbackHandler, trackingId, this, weakThis = weak_from_this()](int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData) {
        RETURN_FROM_LAMBDA_IF_DEAD(weakThis);
        SERVICE_LOG_DEBUG("receive header, status code: " << statusCode << ", trackingId: " << trackingId);
        callbackHandler->setResponseHeader(statusCode, headers, errorData);
    };

    auto curlBodyCallback = [callbackHandler, trackingId, this, weakThis = weak_from_this()](const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished){
        RETURN_FROM_LAMBDA_IF_DEAD(weakThis);
        SERVICE_LOG_DEBUG("receive body data, size: " << buffer.size() << ", isFinished: "<< isFinished << ", trackingId: " << trackingId);
        callbackHandler->appendResponseBody(buffer, isFinished);
    };

    auto curlCompletionCallback = [callbackHandler, trackingId, this, weakThis = weak_from_this()](const ucf::utilities::network::http::HttpResponseMetrics& metrics){
        RETURN_FROM_LAMBDA_IF_DEAD(weakThis);
        SERVICE_LOG_DEBUG("receive completion callback" << ", trackingId: " << trackingId);
        if (callbackHandler->shouldRetryRequest())
        {
            sendHttpRequest(callbackHandler->prepareRetryRequest(), callbackHandler);
        }
        else
        {
            callbackHandler->completeResponse(metrics);
        }
    };
    mDataPrivate->getLibCurlClient()->makeGenericRequest(httpRequest, curlHeaderCallback, curlBodyCallback, curlCompletionCallback);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpManager Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}