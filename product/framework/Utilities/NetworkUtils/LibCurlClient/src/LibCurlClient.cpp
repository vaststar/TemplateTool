#include <ucf/Utilities/NetworkUtils/LibCurlClient/LibCurlClient.h>

#include <mutex>
#include <memory>
#include <filesystem>
#include <ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h>

#include "LibCurlClientLogger.h"
#include "LibCurlMultiHandle.h"
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
    void ensureStarted();
    void stop();
    std::shared_ptr<LibCurlEasyHandle> buildEasyHandle(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest, ucf::utilities::network::http::HttpHeaderCallback headerCallback, ucf::utilities::network::http::HttpBodyCallback bodyCallback, ucf::utilities::network::http::HttpCompletionCallback completionCallback) const;
    void insertEasyHandle(std::shared_ptr<LibCurlEasyHandle> handle);
    bool cancelRequest(const std::string& requestId);
private:
    ucf::utilities::network::http::NetworkHttpHeaders buildHeaders(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest) const;
private:
    std::unique_ptr<LibCurlMultiHandleManager> mMultiHandleManager;
    std::once_flag start_flag;
    std::once_flag stop_flag;

};

LibCurlClient::DataPrivate::DataPrivate() 
    : mMultiHandleManager(std::make_unique<LibCurlMultiHandleManager>())
{
}

void LibCurlClient::DataPrivate::ensureStarted()
{
    std::call_once(start_flag, [this]() {
        LIBCURL_LOG_DEBUG("auto starting service");
        mMultiHandleManager->runLoop();
    });
}

void LibCurlClient::DataPrivate::stop()
{
    LIBCURL_LOG_DEBUG("start stop service");
    std::call_once(stop_flag, [this]() {
        LIBCURL_LOG_DEBUG("stop service");
        mMultiHandleManager->stopLoop();
        LIBCURL_LOG_DEBUG("stop service done");
    });
    LIBCURL_LOG_DEBUG("finish stop service done");
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

std::shared_ptr<LibCurlEasyHandle> LibCurlClient::DataPrivate::buildEasyHandle(const ucf::utilities::network::http::NetworkHttpRequest& httpRequest, ucf::utilities::network::http::HttpHeaderCallback headerCallback, ucf::utilities::network::http::HttpBodyCallback bodyCallback, ucf::utilities::network::http::HttpCompletionCallback completionCallback) const
{
    auto easyHandle = std::make_shared<LibCurlEasyHandle>(headerCallback, bodyCallback, completionCallback);
    easyHandle->setHttpMethod(httpRequest.getRequestMethod());
    easyHandle->setURI(httpRequest.getRequestUri());
    easyHandle->setHeaders(buildHeaders(httpRequest));
    easyHandle->setTrackingId(httpRequest.getTrackingId());
    easyHandle->setRequestId(httpRequest.getRequestId());
    easyHandle->setTimeout(httpRequest.getTimeout());
    easyHandle->setCommonOptions();
    // easyHandle->enableCURLDebugPrint();

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
    ensureCurlGlobalInit();
    LIBCURL_LOG_DEBUG("create LibCurlClient, "<<this);
}

LibCurlClient::~LibCurlClient()
{
    LIBCURL_LOG_DEBUG("delete LibCurlClient, " << this);
    mDataPrivate->stop();
    LIBCURL_LOG_DEBUG("delete LibCurlClient done, " << this);
}

void LibCurlClient::makeGenericRequest(const ucf::utilities::network::http::NetworkHttpRequest& request, ucf::utilities::network::http::HttpHeaderCallback headerCallback, ucf::utilities::network::http::HttpBodyCallback bodyCallback, ucf::utilities::network::http::HttpCompletionCallback completionCallback)
{
    mDataPrivate->ensureStarted();
    LIBCURL_LOG_DEBUG("make request, requestId: " << request.getRequestId() << ", trackingId: " << request.getTrackingId());
    auto easyHandle = mDataPrivate->buildEasyHandle(request, headerCallback, bodyCallback, completionCallback);
    mDataPrivate->insertEasyHandle(easyHandle);
}

bool LibCurlClient::cancelRequest(const std::string& requestId)
{
    LIBCURL_LOG_INFO("[CANCEL_REQUEST] requestId: " << requestId);
    bool result = mDataPrivate->cancelRequest(requestId);
    if (result) {
        LIBCURL_LOG_INFO("[CANCEL_REQUEST] requestId: " << requestId << " cancelled successfully");
    } else {
        LIBCURL_LOG_WARN("[CANCEL_REQUEST] requestId: " << requestId << " not found or already completed");
    }
    return result;
}

bool LibCurlClient::DataPrivate::cancelRequest(const std::string& requestId)
{
    return mMultiHandleManager->cancelRequest(requestId);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish LibCurlClient Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}