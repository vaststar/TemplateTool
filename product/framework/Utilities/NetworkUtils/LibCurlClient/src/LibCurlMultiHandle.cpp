#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <cstdlib>

#include <curl/curl.h>

#include "LibCurlMultiHandle.h"
#include "LibCurlClientLogger.h"
#include "LibCurlEasyHandle.h"

namespace ucf::utilities::network::libcurl{

namespace {
    std::once_flag g_curlInitFlag;
    std::atomic<bool> g_curlInitialized{false};
    
    // Connection pool limits
    constexpr long kMaxTotalConnections = 50;   // Max simultaneous connections (queued if exceeded)
    constexpr long kMaxHostConnections = 6;     // Max connections per host (HTTP/1.1 browser standard)
    constexpr long kMaxConnectionCache = 100;   // Keep-alive connection cache size
}

void ensureCurlGlobalInit() {
    std::call_once(g_curlInitFlag, []() {
        LIBCURL_LOG_DEBUG("curl_global_init called");
        if (curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK) {
            g_curlInitialized.store(true, std::memory_order_release);
            std::atexit([]() {
                LIBCURL_LOG_DEBUG("curl_global_cleanup called");
                curl_global_cleanup();
            });
        } else {
            LIBCURL_LOG_ERROR("curl_global_init failed");
        }
    });
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class LibCurlMultiHandle::DataPrivate
{
public:
    DataPrivate();
    ~DataPrivate();
    int addEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle);
    int removeEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle);
    int perform(int* count);
    int poll(int timeout_ms);
    void completeCurrentRequest();
    void stop();
    bool isStopped();
    void cancelAllPendingRequests();
    bool cancelRequest(const std::string& requestId);
private:
    CURLMsg* read();
    std::shared_ptr<LibCurlEasyHandle> getEasyHanleFromCURL(CURL* handle);
private:
    std::mutex mCURLAccess;
    CURLM* mHandle;

    std::mutex mRequestsAccess;
    std::vector<std::shared_ptr<LibCurlEasyHandle>> mRequests;

    std::atomic_bool mStop;
    std::atomic<int> mLastRunningCount{0};  // Cache running count for logging
};

LibCurlMultiHandle::DataPrivate::DataPrivate()
    : mHandle(curl_multi_init())
    , mStop(false)
{
    if (mHandle) {
        curl_multi_setopt(mHandle, CURLMOPT_MAX_TOTAL_CONNECTIONS, kMaxTotalConnections);
        curl_multi_setopt(mHandle, CURLMOPT_MAX_HOST_CONNECTIONS, kMaxHostConnections);
        curl_multi_setopt(mHandle, CURLMOPT_MAXCONNECTS, kMaxConnectionCache);
    }
}

LibCurlMultiHandle::DataPrivate::~DataPrivate()
{
    if (mHandle)
    {
        curl_multi_cleanup(mHandle);
    }
}

int LibCurlMultiHandle::DataPrivate::addEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle)
{
    size_t totalCount;
    {
        std::scoped_lock lo(mRequestsAccess);
        mRequests.push_back(easyHandle);
        totalCount = mRequests.size();
    }
    
    int running = mLastRunningCount.load(std::memory_order_acquire);
    int queued = (std::max)(0, static_cast<int>(totalCount) - running);
    LIBCURL_LOG_INFO("[REQUEST_ADDED] total=" << totalCount << ", running=" << running << ", queued=" << queued);
    
    std::scoped_lock lo(mCURLAccess);
    auto code = curl_multi_add_handle(mHandle, easyHandle->getHandle());
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("add easy handle error: " << curl_multi_strerror(code));
    }
    LIBCURL_LOG_DEBUG("add easy handle");
    return code;
}

int LibCurlMultiHandle::DataPrivate::removeEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle)
{
    size_t totalCount = 0;
    if (easyHandle)
    {
        std::scoped_lock lo(mRequestsAccess);
        std::erase(mRequests, easyHandle);
        totalCount = mRequests.size();
    }

    int running = mLastRunningCount.load(std::memory_order_acquire);
    int queued = (std::max)(0, static_cast<int>(totalCount) - running);
    LIBCURL_LOG_INFO("[REQUEST_COMPLETED] total=" << totalCount << ", running=" << running << ", queued=" << queued);

    std::scoped_lock lo(mCURLAccess);
    auto code = curl_multi_remove_handle(mHandle, easyHandle->getHandle());
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("remove easy handle error: " << curl_multi_strerror(code));
    }
    LIBCURL_LOG_DEBUG("remove easy handle");
    return code;
}

int LibCurlMultiHandle::DataPrivate::perform(int* count)
{
    std::scoped_lock lo(mCURLAccess);
    auto code = curl_multi_perform(mHandle, count);
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("multi handle perform error: " << curl_multi_strerror(code));
    }
    mLastRunningCount.store(*count, std::memory_order_release);
    return code;
}

int LibCurlMultiHandle::DataPrivate::poll(int timeout_ms)
{
    auto code = curl_multi_poll(mHandle, nullptr, 0, timeout_ms, nullptr);
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_INFO("multi handle perform error: " << curl_multi_strerror(code));
    }
    return code;
}

CURLMsg* LibCurlMultiHandle::DataPrivate::read()
{
    std::scoped_lock lo(mCURLAccess);
    int count = -1;
    return curl_multi_info_read(mHandle, &count);
}

std::shared_ptr<LibCurlEasyHandle> LibCurlMultiHandle::DataPrivate::getEasyHanleFromCURL(CURL* handle)
{
    std::scoped_lock lo(mRequestsAccess);
    auto iter = std::find_if(mRequests.cbegin(), mRequests.cend(), [handle](const auto& request){
        return request->getHandle() == handle;
    });

    if (iter != mRequests.end())
    {
        return *iter;
    }
    return nullptr;
}

void LibCurlMultiHandle::DataPrivate::completeCurrentRequest()
{
    CURLMsg* msg = nullptr;
    while((msg = read()))
    {
        if (CURLMSG_DONE == msg->msg)
        {
            if (auto libCurlHandle = getEasyHanleFromCURL(msg->easy_handle))
            {
                libCurlHandle->finishHandle(msg->data.result);
                removeEasyHandle(libCurlHandle);
            }
            else
            {
                LIBCURL_LOG_WARN("cannot find the EasyHandle");
                std::scoped_lock lo(mCURLAccess);
                curl_multi_remove_handle(mHandle, msg->easy_handle);
            }
        }
        else
        {
            LIBCURL_LOG_WARN("Invalid message's code: " << msg->msg);
        }
    }
}

void LibCurlMultiHandle::DataPrivate::stop()
{
    mStop.store(true, std::memory_order_release);
    if (mHandle)
    {
        curl_multi_wakeup(mHandle);
    }
}

bool LibCurlMultiHandle::DataPrivate::isStopped()
{
    return mStop.load(std::memory_order_acquire);
}

void LibCurlMultiHandle::DataPrivate::cancelAllPendingRequests()
{
    std::vector<std::shared_ptr<LibCurlEasyHandle>> requestsToCancel;
    {
        std::scoped_lock lo(mRequestsAccess);
        requestsToCancel = mRequests;
        mRequests.clear();
    }

    for(auto& request: requestsToCancel)
    {
        {
            std::scoped_lock lo(mCURLAccess);
            curl_multi_remove_handle(mHandle, request->getHandle());
        }
        request->finishHandle(CURLE_ABORTED_BY_CALLBACK);
        LIBCURL_LOG_INFO("Request canceled during shutdown:")
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start LibCurlMultiHandle Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

LibCurlMultiHandle::LibCurlMultiHandle()
    : mDataPrivate(std::make_unique<DataPrivate>())
{
    LIBCURL_LOG_DEBUG("create multi handle");
}

LibCurlMultiHandle::~LibCurlMultiHandle()
{
    LIBCURL_LOG_DEBUG("destory multi handle");
}

int LibCurlMultiHandle::addEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle)
{
    return mDataPrivate->addEasyHandle(easyHandle);
}

int LibCurlMultiHandle::removeEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle)
{
    return mDataPrivate->removeEasyHandle(easyHandle);
}

void LibCurlMultiHandle::performRequests()
{
    int runningHandles = 0;
    do
    {
        if (mDataPrivate->isStopped())
        {
            LIBCURL_LOG_WARN("stop signal received, cancaling:" << runningHandles << "pending requests");
            mDataPrivate->cancelAllPendingRequests();
            break;
        }

        if (auto code = mDataPrivate->perform(&runningHandles); CURLM_OK !=  code)
        {
            LIBCURL_LOG_WARN("multi perform failed:" << code);
            break;
        }

        mDataPrivate->completeCurrentRequest();

        if (runningHandles > 0)
        {
            if (auto code = mDataPrivate->poll(100); CURLM_OK !=  code)
            {
                LIBCURL_LOG_WARN("multi poll failed:" << code);
                break;
            }
        }
    }while(runningHandles > 0);
}

void LibCurlMultiHandle::stop()
{
    LIBCURL_LOG_INFO("");
    mDataPrivate->stop();
}

bool LibCurlMultiHandle::cancelRequest(const std::string& requestId)
{
    return mDataPrivate->cancelRequest(requestId);
}

bool LibCurlMultiHandle::DataPrivate::cancelRequest(const std::string& requestId)
{
    std::shared_ptr<LibCurlEasyHandle> handleToCancel;
    size_t totalCount = 0;
    {
        std::scoped_lock lo(mRequestsAccess);
        auto iter = std::find_if(mRequests.begin(), mRequests.end(),
            [&requestId](const auto& request) {
                return request->getRequestId() == requestId;
            });
        
        if (iter == mRequests.end()) {
            LIBCURL_LOG_WARN("[REQUEST_CANCEL] requestId=" << requestId << " not found");
            return false;
        }
        
        handleToCancel = *iter;
        mRequests.erase(iter);
        totalCount = mRequests.size();
    }

    {
        std::scoped_lock lo(mCURLAccess);
        curl_multi_remove_handle(mHandle, handleToCancel->getHandle());
    }
    
    int running = mLastRunningCount.load(std::memory_order_acquire);
    int queued = (std::max)(0, static_cast<int>(totalCount) - running);
    LIBCURL_LOG_INFO("[REQUEST_CANCELLED] requestId=" << requestId << ", total=" << totalCount << ", running=" << running << ", queued=" << queued);
    
    handleToCancel->finishHandle(CURLE_ABORTED_BY_CALLBACK);
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start LibCurlMultiHandle Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}