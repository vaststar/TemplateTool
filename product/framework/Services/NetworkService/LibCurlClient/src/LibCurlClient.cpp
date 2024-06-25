#include <memory>
#include <ucf/Services/NetworkService/LibCurlClient/LibCurlClient.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>

#include "LibCurlClientLogger.h"
#include "LibCurlMultiHandleManager.h"
#include "LibCurlEasyHandle.h"

namespace ucf::service::network::libcurl{
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
    std::shared_ptr<LibCurlEasyHandle> buildEasyHandle(const ucf::service::network::http::NetworkHttpRequest& httpRequest, const ucf::service::network::http::HttpHeaderCallback& headerCallback, const ucf::service::network::http::HttpBodyCallback& bodyCallback, const ucf::service::network::http::HttpCompletionCallback& completionCallback) const;
    void insertEasyHandle(std::shared_ptr<LibCurlEasyHandle> handle);
private:
    ucf::service::network::http::NetworkHttpHeaders buildHeaders(const ucf::service::network::http::NetworkHttpRequest& httpRequest) const;
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

ucf::service::network::http::NetworkHttpHeaders LibCurlClient::DataPrivate::buildHeaders(const ucf::service::network::http::NetworkHttpRequest& httpRequest) const
{
    auto requestHeaders = httpRequest.getRequestHeaders();
    requestHeaders.emplace_back("TrackingID", httpRequest.getTrackingId());
    return requestHeaders;
}

std::shared_ptr<LibCurlEasyHandle> LibCurlClient::DataPrivate::buildEasyHandle(const ucf::service::network::http::NetworkHttpRequest& httpRequest, const ucf::service::network::http::HttpHeaderCallback& headerCallback, const ucf::service::network::http::HttpBodyCallback& bodyCallback, const ucf::service::network::http::HttpCompletionCallback& completionCallback) const
{
    auto easyHandle = std::make_shared<LibCurlEasyHandle>(headerCallback, bodyCallback, completionCallback);
    easyHandle->setHttpMethod(httpRequest.getRequestMethod());
    easyHandle->setURI(httpRequest.getRequestUri());
    easyHandle->setHeaders(buildHeaders(httpRequest));
    easyHandle->setTrackingId(httpRequest.getTrackingId());
    easyHandle->setTimeout(httpRequest.getTimeout());
    easyHandle->setCommonOptions();
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
    LIBCURL_LOG_DEBUG(""<<this);
    startService();
}

LibCurlClient::~LibCurlClient()
{
    LIBCURL_LOG_DEBUG("" << this);
    stopService();
}

void LibCurlClient::startService()
{
    LIBCURL_LOG_DEBUG("");
    mDataPrivate->start();
}

void LibCurlClient::stopService()
{
    LIBCURL_LOG_DEBUG("");
    mDataPrivate->stop();
}

void LibCurlClient::makeGenericRequest(const ucf::service::network::http::NetworkHttpRequest& request, const ucf::service::network::http::HttpHeaderCallback& headerCallback, const ucf::service::network::http::HttpBodyCallback& bodyCallback, const ucf::service::network::http::HttpCompletionCallback& completionCallback)
{
    LIBCURL_LOG_DEBUG("");
    auto easyHandle = mDataPrivate->buildEasyHandle(request, headerCallback, bodyCallback, completionCallback);
    mDataPrivate->insertEasyHandle(easyHandle);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish LibCurlClient Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}