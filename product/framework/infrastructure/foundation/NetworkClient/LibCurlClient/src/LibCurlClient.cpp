#include <ucf/infrastructure/NetworkClient/LibCurlClient/LibCurlClient.h>

#include <mutex>
#include <memory>
#include <filesystem>
#include <ucf/infrastructure/NetworkClient/NetworkModelTypes/http/NetworkHttpRequest.h>

#include "LibCurlClientLogger.h"
#include "LibCurlMultiHandle.h"
#include "LibCurlMultiHandleManager.h"
#include "LibCurlEasyHandle.h"

namespace ucf::infrastructure::network::libcurl{
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
    std::shared_ptr<LibCurlEasyHandle> buildEasyHandle(const ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest, ucf::infrastructure::network::http::HttpHeaderCallback headerCallback, ucf::infrastructure::network::http::HttpBodyCallback bodyCallback, ucf::infrastructure::network::http::HttpCompletionCallback completionCallback) const;
    void insertEasyHandle(std::shared_ptr<LibCurlEasyHandle> handle);
    bool cancelRequest(const std::string& requestId);
private:
    ucf::infrastructure::network::http::NetworkHttpHeaders buildHeaders(const ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const;
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
        LIBCURL_LOG_DEBUG(
            "LibCurlClient automatic startup started, address: " << this);

        mMultiHandleManager->runLoop();

        LIBCURL_LOG_DEBUG(
            "LibCurlClient automatic startup finished, address: " << this);
    });
}

void LibCurlClient::DataPrivate::stop()
{
    LIBCURL_LOG_DEBUG(
        "LibCurlClient DataPrivate shutdown started, address: " << this);

    std::call_once(stop_flag, [this]() {
        LIBCURL_LOG_DEBUG(
            "LibCurlClient one-time shutdown started, address: " << this);

        mMultiHandleManager->stopLoop();

        LIBCURL_LOG_DEBUG(
            "LibCurlClient one-time shutdown finished, address: " << this);
    });

    LIBCURL_LOG_DEBUG(
        "LibCurlClient DataPrivate shutdown finished, address: " << this);
}

void LibCurlClient::DataPrivate::insertEasyHandle(std::shared_ptr<LibCurlEasyHandle> handle)
{
    mMultiHandleManager->insert(handle);
}

ucf::infrastructure::network::http::NetworkHttpHeaders LibCurlClient::DataPrivate::buildHeaders(const ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest) const
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

std::shared_ptr<LibCurlEasyHandle> LibCurlClient::DataPrivate::buildEasyHandle(const ucf::infrastructure::network::http::NetworkHttpRequest& httpRequest, ucf::infrastructure::network::http::HttpHeaderCallback headerCallback, ucf::infrastructure::network::http::HttpBodyCallback bodyCallback, ucf::infrastructure::network::http::HttpCompletionCallback completionCallback) const
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
    case ucf::infrastructure::network::http::NetworkHttpPayloadType::String:
        easyHandle->setRequestDataString(httpRequest.getPayloadString());
        break;
    case ucf::infrastructure::network::http::NetworkHttpPayloadType::Memory:
        easyHandle->setRequestDataBuffer(httpRequest.getPayloadMemoryBuffer(), httpRequest.getProgressFunction());
        easyHandle->setInFileSizeLarge(httpRequest.getPayloadSize());
        break;
    case ucf::infrastructure::network::http::NetworkHttpPayloadType::File:
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
    LIBCURL_LOG_DEBUG(
        "LibCurlClient constructed, address: " << this);
}

LibCurlClient::~LibCurlClient()
{
    LIBCURL_LOG_DEBUG(
        "LibCurlClient destruction started, address: " << this);

    mDataPrivate->stop();
    mDataPrivate.reset();

    LIBCURL_LOG_DEBUG(
        "LibCurlClient destruction finished, address: " << this);
}

void LibCurlClient::makeGenericRequest(const ucf::infrastructure::network::http::NetworkHttpRequest& request, ucf::infrastructure::network::http::HttpHeaderCallback headerCallback, ucf::infrastructure::network::http::HttpBodyCallback bodyCallback, ucf::infrastructure::network::http::HttpCompletionCallback completionCallback)
{
    LIBCURL_LOG_DEBUG(
        "Network request submission started, requestId: "
        << request.getRequestId()
        << ", trackingId: "
        << request.getTrackingId());

    mDataPrivate->ensureStarted();

    auto easyHandle = mDataPrivate->buildEasyHandle(request, headerCallback, bodyCallback, completionCallback);
    mDataPrivate->insertEasyHandle(easyHandle);

    LIBCURL_LOG_DEBUG(
        "Network request submission finished, requestId: "
        << request.getRequestId()
        << ", trackingId: "
        << request.getTrackingId());
}

bool LibCurlClient::cancelRequest(const std::string& requestId)
{
    LIBCURL_LOG_INFO(
        "Network request cancellation started, requestId: " << requestId);

    const bool result = mDataPrivate->cancelRequest(requestId);

    if (result)
    {
        LIBCURL_LOG_INFO(
            "Network request cancellation finished, requestId: " << requestId);
    }
    else
    {
        LIBCURL_LOG_WARN(
            "Network request cancellation failed: request was not found "
            "or already completed, requestId: "
            << requestId);
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
