#include "NetworkHttpManager.h"
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>
#include <ucf/Utilities/NetworkUtils/LibCurlClient/LibCurlClient.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h>

#include "NetworkHttpHandler/NetworkHttpRestHandler.h"
#include "NetworkHttpHandler/NetworkHttpRawHandler.h"
#include "NetworkHttpHandler/NetworkHttpDownloadToContentHandler.h"

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
    auto httpHandler = std::make_shared<NetworkHttpRestHandler>(restRequest, restResponseCallback);
    sendHttpRequest(httpHandler, location);
}

void NetworkHttpManager::sendHttpRawRequest(const ucf::service::network::http::HttpRawRequest& rawRequest, const ucf::service::network::http::HttpRawResponseCallbackFunc& rawResponseCallback, const std::source_location location)
{
    auto httpHandler = std::make_shared<NetworkHttpRawHandler>(rawRequest, rawResponseCallback);
    sendHttpRequest(httpHandler, location);
}

void NetworkHttpManager::downloadContentToMemory(const ucf::service::network::http::HttpDownloadToContentRequest& downloadRequest, const ucf::service::network::http::HttpDownloadToContentResponseCallbackFunc& downloadResponseCallback, const std::source_location location)
{
    auto httpHandler = std::make_shared<NetworkHttpDownloadToContentHandler>(downloadRequest, downloadResponseCallback);
    sendHttpRequest(httpHandler, location);
}

void NetworkHttpManager::sendHttpRequest(std::shared_ptr<INetworkHttpHandler> httpHandler, const std::source_location location)
{
   SERVICE_LOG_DEBUG("about making http request:" << httpHandler->getHttpRequest().toString() <<", from: " 
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

    std::string trackingId = httpHandler->getHttpRequest().getTrackingId();
    std::string requestId = httpHandler->getHttpRequest().getRequestId();
    auto curlHeaderCallback = [httpHandler, requestId, trackingId, this, weakThis = weak_from_this()](int statusCode, const ucf::utilities::network::http::NetworkHttpHeaders& headers, std::optional<ucf::utilities::network::http::ResponseErrorStruct> errorData) {
        RETURN_FROM_LAMBDA_IF_DEAD(weakThis);
        SERVICE_LOG_DEBUG("receive header, status code: " << statusCode << ", requestId: " << requestId << ", trackingId: " << trackingId);
        httpHandler->setResponseHeader(statusCode, headers, errorData);
    };

    auto curlBodyCallback = [httpHandler, requestId, trackingId, this, weakThis = weak_from_this()](const ucf::utilities::network::http::ByteBuffer& buffer, bool isFinished){
        RETURN_FROM_LAMBDA_IF_DEAD(weakThis);
        SERVICE_LOG_DEBUG("receive body data, size: " << buffer.size() << ", isFinished: "<< isFinished<< ", requestId: "<< requestId<< ", trackingId: " << trackingId);
        httpHandler->appendResponseBody(buffer, isFinished);
    };

    auto curlCompletionCallback = [httpHandler, requestId, trackingId, this, weakThis = weak_from_this()](const ucf::utilities::network::http::HttpResponseMetrics& metrics){
        RETURN_FROM_LAMBDA_IF_DEAD(weakThis);
        SERVICE_LOG_DEBUG("receive completion callback"<< ", requestId: "<< requestId<< ", trackingId: "<< trackingId);
        if (httpHandler->shouldRedirectRequest())
        {
            SERVICE_LOG_DEBUG("will redirect "<< ", requestId: "<< requestId<< ", trackingId: "<< trackingId);
            httpHandler->prepareRedirectRequest();
            sendHttpRequest(httpHandler);
        }
        else if (httpHandler->shoudRetryRequest())
        {
            SERVICE_LOG_DEBUG("will retry after" << httpHandler->getRetryAfterMillSecs()<< "ms, requestId: "<< requestId<< ", trackingId: "<< trackingId);
            httpHandler->prepareRetryRequest();
            sendHttpRequest(httpHandler);
        }
        else
        {
            httpHandler->completeResponse(metrics);
        }
    };
    mDataPrivate->getLibCurlClient()->makeGenericRequest(httpHandler->getHttpRequest(), curlHeaderCallback, curlBodyCallback, curlCompletionCallback);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkHttpManager Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}