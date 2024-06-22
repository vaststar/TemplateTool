#include <memory>
#include <ucf/Services/NetworkService/LibCurlClient/LibCurlClient.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpRequest.h>
#include <ucf/Services/NetworkService/NetworkModelTypes/Http/NetworkHttpTypes.h>

#include "LibCurlClientLogger.h"
#include "LibCurlMultiHandleManager.h"
#include "LibCurlEasyHandle.h"

namespace ucf::network::libcurl{
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
    std::shared_ptr<LibCurlEasyHandle> buildEasyHandle(const ucf::network::http::NetworkHttpRequest& httpRequest, const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback) const;
    void insertEasyHandle(std::shared_ptr<LibCurlEasyHandle> handle);
private:
    ucf::network::http::NetworkHttpHeaders buildHeaders(const ucf::network::http::NetworkHttpRequest& httpRequest) const;
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

ucf::network::http::NetworkHttpHeaders LibCurlClient::DataPrivate::buildHeaders(const ucf::network::http::NetworkHttpRequest& httpRequest) const
{
    auto requestHeaders = httpRequest.getRequestHeaders();
    requestHeaders.emplace_back("TrackingID", httpRequest.getTrackingId());
    return requestHeaders;
}

std::shared_ptr<LibCurlEasyHandle> LibCurlClient::DataPrivate::buildEasyHandle(const ucf::network::http::NetworkHttpRequest& httpRequest, const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback) const
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

void LibCurlClient::makeGenericRequest(const ucf::network::http::NetworkHttpRequest& request, const ucf::network::http::HttpHeaderCallback& headerCallback, const ucf::network::http::HttpBodyCallback& bodyCallback, const ucf::network::http::HttpCompletionCallback& completionCallback)
{
    LIBCURL_LOG_DEBUG("");
    auto easyHandle = mDataPrivate->buildEasyHandle(request, headerCallback, bodyCallback, completionCallback);
    mDataPrivate->insertEasyHandle(easyHandle);

    
// CURL *curl;
//     CURLcode res;
    
//     curl = curl_easy_init();
//     if (curl) {
//         curl_easy_setopt(curl, CURLOPT_URL, "http://www.example.com");
//         /* example.com is redirected, so we tell libcurl to follow redirection */
//         curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
//         res = curl_easy_perform(curl);
        
//         /* Check for errors */
//         if (res != CURLE_OK) {
//             fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
//         }
        
//         /* always cleanup */
//         curl_easy_cleanup(curl);
//     }

}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish LibCurlClient Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}