#include <memory>
#include <filesystem>
#include <ucf/Utilities/NetworkUtils/LibCurlClient/LibCurlClient.h>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>

#include "LibCurlClientLogger.h"
#include "LibCurlMultiHandleManager.h"
#include "LibCurlEasyHandle.h"

namespace ucf::utilities::network::libcurl{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class LibCurlClient::DataPrivate
{
public:
    DataPrivate();
    void start();
    void stop();
    std::shared_ptr<LibCurlEasyHandle> buildEasyHandle(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest, const ucf::utilities::network::http::HttpHeaderCallback& headerCallback, const ucf::utilities::network::http::HttpBodyCallback& bodyCallback, const ucf::utilities::network::http::HttpCompletionCallback& completionCallback) const;
    void insertEasyHandle(std::shared_ptr<LibCurlEasyHandle> handle);
private:
    ucf::utilities::network::http::NetworkHttpHeaders buildHeaders(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const;
private:
    std::unique_ptr<LibCurlMultiHandleManager> mMultiHandleManager;
};

LibCurlClient::DataPrivate::DataPrivate()
    : mMultiHandleManager(std::make_unique<LibCurlMultiHandleManager>())
{
}

void LibCurlClient::DataPrivate::start()
{
    mMultiHandleManager->runLoop();
}

void LibCurlClient::DataPrivate::stop()
{
    mMultiHandleManager->stopLoop();
}

void LibCurlClient::DataPrivate::insertEasyHandle(std::shared_ptr<LibCurlEasyHandle> handle)
{
    mMultiHandleManager->insert(handle);
}

ucf::utilities::network::http::NetworkHttpHeaders LibCurlClient::DataPrivate::buildHeaders(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const
{
    auto requestHeaders = httpRequest.getRequestHeaders();
    requestHeaders.emplace_back("Expect", "");
    requestHeaders.emplace_back("TrackingID", httpRequest.getTrackingId());
    if (auto contentLength = httpRequest.getPayloadSize(); contentLength > 0)
    {
        requestHeaders.emplace_back("Content-Length", std::to_string(contentLength));
    }
    return requestHeaders;
}

std::shared_ptr<LibCurlEasyHandle> LibCurlClient::DataPrivate::buildEasyHandle(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest, const ucf::utilities::network::http::HttpHeaderCallback& headerCallback, const ucf::utilities::network::http::HttpBodyCallback& bodyCallback, const ucf::utilities::network::http::HttpCompletionCallback& completionCallback) const
{
    auto easyHandle = std::make_shared<LibCurlEasyHandle>(headerCallback, bodyCallback, completionCallback);
    easyHandle->setHttpMethod(httpRequest.getRequestMethod());
    easyHandle->setURI(httpRequest.getRequestUri());
    easyHandle->setHeaders(buildHeaders(httpRequest));
    easyHandle->setTrackingId(httpRequest.getTrackingId());
    easyHandle->setTimeout(httpRequest.getTimeout());
    easyHandle->setCommonOptions();
    easyHandle->enableCURLDebugPrint();

    switch (httpRequest.getPayloadType())
    {
    case ucf::utilities::network::http::NetworkHttpPayloadType::String:
        easyHandle->setRequestDataString(httpRequest.getPayloadString());
        break;
    case ucf::utilities::network::http::NetworkHttpPayloadType::Memory:
        easyHandle->setRequestDataBuffer(httpRequest.getPayloadMemoryBuffer(), httpRequest.getProgressFunction());
        easyHandle->setInFileSizeLarge(httpRequest.getPayloadSize());
        break;
    case ucf::utilities::network::http::NetworkHttpPayloadType::File:
        easyHandle->setRequestDataFile(httpRequest.getPayloadFilePath(), httpRequest.getProgressFunction());
        easyHandle->setInFileSizeLarge(httpRequest.getPayloadSize());
        break;
    default:
        break;
    }
    return easyHandle;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start LibCurlClient Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
LibCurlClient::LibCurlClient()
    : mDataPrivate(std::make_unique<DataPrivate>())
{
    LIBCURL_LOG_DEBUG("create LibCurlClient, "<<this);
    startService();
    LIBCURL_LOG_DEBUG("create LibCurlClient done, "<<this);
}

LibCurlClient::~LibCurlClient()
{
    LIBCURL_LOG_DEBUG("delete LibCurlClient, " << this);
    stopService();
    LIBCURL_LOG_DEBUG("delete LibCurlClient done, " << this);
}

void LibCurlClient::startService()
{
    LIBCURL_LOG_DEBUG("start service, " << this);
    mDataPrivate->start();
    LIBCURL_LOG_DEBUG("start service done, " << this);
}

void LibCurlClient::stopService()
{
    LIBCURL_LOG_DEBUG("stop service, " << this);
    mDataPrivate->stop();
    LIBCURL_LOG_DEBUG("stop service done, " << this);
}

void LibCurlClient::makeGenericRequest(const ucf::utilities::network::http::NetworkHttpRequest& request, const ucf::utilities::network::http::HttpHeaderCallback& headerCallback, const ucf::utilities::network::http::HttpBodyCallback& bodyCallback, const ucf::utilities::network::http::HttpCompletionCallback& completionCallback)
{
    LIBCURL_LOG_DEBUG("make request, requestId: " << request.getRequestId() << ", trackingId: " << request.getTrackingId());
    auto easyHandle = mDataPrivate->buildEasyHandle(request, headerCallback, bodyCallback, completionCallback);
    mDataPrivate->insertEasyHandle(easyHandle);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish LibCurlClient Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}