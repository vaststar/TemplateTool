#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <unordered_set>

#include <curl/curl.h>

#include "LibCurlMultiHandle.h"
#include "LibCurlClientLogger.h"
#include "LibCurlEasyHandle.h"

namespace ucf::infrastructure::network::libcurl{

namespace {
    // Connection pool limits
    constexpr long kMaxTotalConnections = 50;   // Max simultaneous connections (queued if exceeded)
    constexpr long kMaxHostConnections = 6;     // Max connections per host (HTTP/1.1 browser standard)
    constexpr long kMaxConnectionCache = 100;   // Keep-alive connection cache size

    thread_local const void* gCurlOperationOwner = nullptr;

    class CurlOperationScope final
    {
    public:
        explicit CurlOperationScope(const void* owner)
            : mPreviousOwner(gCurlOperationOwner)
        {
            gCurlOperationOwner = owner;
        }

        ~CurlOperationScope()
        {
            gCurlOperationOwner = mPreviousOwner;
        }

    private:
        const void* mPreviousOwner;
    };
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
    void drainCanceledRequests();
private:
    std::mutex mCURLAccess;
    CURLM* mHandle;

    std::mutex mRequestsAccess;
    std::vector<std::shared_ptr<LibCurlEasyHandle>> mRequests;
    std::unordered_set<LibCurlEasyHandle*> mAddingRequests;
    std::vector<std::shared_ptr<LibCurlEasyHandle>> mCanceledRequests;

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
    size_t totalCount = 0;
    bool stopped = false;
    {
        std::scoped_lock lo(mRequestsAccess);
        stopped = mStop.load(std::memory_order_acquire);
        if (!stopped)
        {
            mRequests.push_back(easyHandle);
            mAddingRequests.insert(easyHandle.get());
            totalCount = mRequests.size();
        }
    }
    if (stopped)
    {
        easyHandle->finishHandle(CURLE_ABORTED_BY_CALLBACK);
        return CURLM_OK;
    }
    
    int running = mLastRunningCount.load(std::memory_order_acquire);
    int queued = (std::max)(0, static_cast<int>(totalCount) - running);
    LIBCURL_LOG_INFO(
        "Network request queued, requestId: "
        << easyHandle->getRequestId()
        << ", totalCount: "
        << totalCount
        << ", runningCount: "
        << running
        << ", queuedCount: "
        << queued);
    
    CURLMcode code = CURLM_OK;
    bool canceled = false;
    bool addFailed = false;
    {
        std::scoped_lock curlLock(mCURLAccess);
        CurlOperationScope curlScope(this);

        // A cancellation can claim the published request before it reaches curl.
        {
            std::scoped_lock requestsLock(mRequestsAccess);
            auto iter = std::find(mRequests.begin(), mRequests.end(), easyHandle);
            if (iter == mRequests.end() || mStop.load(std::memory_order_acquire))
            {
                canceled = true;
                if (iter != mRequests.end())
                {
                    mRequests.erase(iter);
                }
                mAddingRequests.erase(easyHandle.get());
            }
        }

        if (!canceled)
        {
            code = curl_multi_add_handle(mHandle, easyHandle->getHandle());

            {
                std::scoped_lock requestsLock(mRequestsAccess);
                mAddingRequests.erase(easyHandle.get());
                auto iter = std::find(mRequests.begin(), mRequests.end(), easyHandle);
                canceled = iter == mRequests.end() || mStop.load(std::memory_order_acquire);
                addFailed = code != CURLM_OK && !canceled;
                if ((canceled || addFailed) && iter != mRequests.end())
                {
                    mRequests.erase(iter);
                }
            }

            if (canceled && code == CURLM_OK)
            {
                curl_multi_remove_handle(mHandle, easyHandle->getHandle());
            }
        }
    }

    if (canceled)
    {
        easyHandle->finishHandle(CURLE_ABORTED_BY_CALLBACK);
    }
    else if (addFailed)
    {
        easyHandle->finishHandle(CURLE_FAILED_INIT);
    }

    drainCanceledRequests();

    if (CURLM_OK != code)
    {
        LIBCURL_LOG_ERROR(
            "Network request addition to curl multi handle failed, requestId: "
            << easyHandle->getRequestId()
            << ", errorCode: "
            << static_cast<int>(code)
            << ", error: "
            << curl_multi_strerror(code));
    }
    else
    {
        LIBCURL_LOG_DEBUG(
            "Network request added to curl multi handle, requestId: "
            << easyHandle->getRequestId());
    }
    return code;
}

int LibCurlMultiHandle::DataPrivate::removeEasyHandle(std::shared_ptr<LibCurlEasyHandle> easyHandle)
{
    if (!easyHandle)
    {
        return CURLM_BAD_EASY_HANDLE;
    }

    const auto requestId = easyHandle->getRequestId();
    size_t totalCount = 0;
    bool adding = false;
    {
        std::scoped_lock lo(mRequestsAccess);
        auto iter = std::find(mRequests.begin(), mRequests.end(), easyHandle);
        if (iter == mRequests.end())
        {
            return CURLM_BAD_EASY_HANDLE;
        }
        adding = mAddingRequests.contains(easyHandle.get());
        mRequests.erase(iter);
        totalCount = mRequests.size();
    }

    if (adding)
    {
        // The thread adding this handle owns its eventual curl removal.
        return CURLM_OK;
    }

    int running = mLastRunningCount.load(std::memory_order_acquire);
    int queued = (std::max)(0, static_cast<int>(totalCount) - running);
    LIBCURL_LOG_INFO(
        "Network request completed, requestId: "
        << requestId
        << ", totalCount: "
        << totalCount
        << ", runningCount: "
        << running
        << ", queuedCount: "
        << queued);

    CURLMcode code;
    {
        std::scoped_lock lo(mCURLAccess);
        CurlOperationScope curlScope(this);
        code = curl_multi_remove_handle(mHandle, easyHandle->getHandle());
    }
    drainCanceledRequests();
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_ERROR(
            "Network request removal from curl multi handle failed, requestId: "
            << requestId
            << ", errorCode: "
            << static_cast<int>(code)
            << ", error: "
            << curl_multi_strerror(code));
    }
    else
    {
        LIBCURL_LOG_DEBUG(
            "Network request removed from curl multi handle, requestId: "
            << requestId);
    }
    return code;
}

int LibCurlMultiHandle::DataPrivate::perform(int* count)
{
    CURLMcode code;
    {
        std::scoped_lock lo(mCURLAccess);
        CurlOperationScope curlScope(this);
        code = curl_multi_perform(mHandle, count);
    }
    drainCanceledRequests();
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_ERROR(
            "curl multi perform failed, errorCode: "
            << static_cast<int>(code)
            << ", error: "
            << curl_multi_strerror(code));
    }
    mLastRunningCount.store(*count, std::memory_order_release);
    return code;
}

int LibCurlMultiHandle::DataPrivate::poll(int timeout_ms)
{
    CURLMcode code;
    {
        std::scoped_lock lo(mCURLAccess);
        CurlOperationScope curlScope(this);
        code = curl_multi_poll(mHandle, nullptr, 0, timeout_ms, nullptr);
    }
    drainCanceledRequests();
    if (CURLM_OK != code)
    {
        LIBCURL_LOG_ERROR(
            "curl multi poll failed, errorCode: "
            << static_cast<int>(code)
            << ", error: "
            << curl_multi_strerror(code));
    }
    return code;
}

void LibCurlMultiHandle::DataPrivate::drainCanceledRequests()
{
    // A libcurl body/header callback can call cancelRequest while perform owns
    // mCURLAccess. Defer its curl removal until that operation has returned.
    if (gCurlOperationOwner == this)
    {
        return;
    }

    while (true)
    {
        std::vector<std::shared_ptr<LibCurlEasyHandle>> canceled;
        {
            std::scoped_lock curlLock(mCURLAccess);
            CurlOperationScope curlScope(this);
            {
                std::scoped_lock requestsLock(mRequestsAccess);
                canceled.swap(mCanceledRequests);
            }
            for (const auto& request : canceled)
            {
                curl_multi_remove_handle(mHandle, request->getHandle());
            }
        }
        if (canceled.empty())
        {
            return;
        }
        for (const auto& request : canceled)
        {
            request->finishHandle(CURLE_ABORTED_BY_CALLBACK);
        }
    }
}

void LibCurlMultiHandle::DataPrivate::completeCurrentRequest()
{
    while (true)
    {
        std::shared_ptr<LibCurlEasyHandle> request;
        CURL* curlHandle = nullptr;
        CURLcode result = CURLE_OK;
        CURLMSG messageType{};
        {
            std::scoped_lock curlLock(mCURLAccess);
            CurlOperationScope curlScope(this);
            int count = 0;
            CURLMsg* message = curl_multi_info_read(mHandle, &count);
            if (!message)
            {
                break;
            }

            // curl owns the message. Copy its fields before another thread can
            // remove the easy handle or invalidate the message storage.
            messageType = message->msg;
            curlHandle = message->easy_handle;
            if (messageType == CURLMSG_DONE)
            {
                result = message->data.result;
                {
                    std::scoped_lock requestsLock(mRequestsAccess);
                    auto iter = std::find_if(mRequests.begin(), mRequests.end(),
                        [curlHandle](const auto& candidate) {
                            return candidate->getHandle() == curlHandle;
                        });
                    if (iter != mRequests.end())
                    {
                        request = *iter;
                        mRequests.erase(iter);
                    }
                }
                if (request)
                {
                    curl_multi_remove_handle(mHandle, curlHandle);
                }
            }
        }

        if (messageType == CURLMSG_DONE)
        {
            if (request)
            {
                request->finishHandle(result);
            }
            else
            {
                LIBCURL_LOG_WARN(
                    "Completed curl handle was already claimed or was not tracked, "
                    "curlHandle: "
                    << static_cast<const void*>(curlHandle));
            }
        }
        else
        {
            LIBCURL_LOG_WARN(
                "Unexpected curl multi message received, messageCode: "
                << static_cast<int>(messageType));
        }
        drainCanceledRequests();
    }
    drainCanceledRequests();
}

void LibCurlMultiHandle::DataPrivate::stop()
{
    mStop.store(true, std::memory_order_release);
    if (mHandle)
    {
        curl_multi_wakeup(mHandle);
    }
    // The worker may exit between performRequests calls. Claim outstanding
    // handles here as well so shutdown does not depend on another loop turn.
    cancelAllPendingRequests();
}

bool LibCurlMultiHandle::DataPrivate::isStopped()
{
    return mStop.load(std::memory_order_acquire);
}

void LibCurlMultiHandle::DataPrivate::cancelAllPendingRequests()
{
    {
        std::scoped_lock lo(mRequestsAccess);
        for (const auto& request : mRequests)
        {
            if (!mAddingRequests.contains(request.get()))
            {
                mCanceledRequests.push_back(request);
            }
        }
        mRequests.clear();
    }
    drainCanceledRequests();
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
    LIBCURL_LOG_DEBUG(
        "LibCurlMultiHandle constructed, address: " << this);
}

LibCurlMultiHandle::~LibCurlMultiHandle()
{
    LIBCURL_LOG_DEBUG(
        "LibCurlMultiHandle destroying, address: " << this);
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
            LIBCURL_LOG_WARN(
                "Network request processing stopping: shutdown was requested, "
                "runningRequestCount: "
                << runningHandles);
            mDataPrivate->cancelAllPendingRequests();
            break;
        }

        if (auto code = mDataPrivate->perform(&runningHandles); CURLM_OK !=  code)
        {
            LIBCURL_LOG_WARN(
                "Network request processing stopped: curl multi perform failed, "
                "errorCode: "
                << static_cast<int>(code)
                << ", error: "
                << curl_multi_strerror(static_cast<CURLMcode>(code)));
            break;
        }

        mDataPrivate->completeCurrentRequest();

        if (runningHandles > 0)
        {
            if (auto code = mDataPrivate->poll(100); CURLM_OK !=  code)
            {
                LIBCURL_LOG_WARN(
                    "Network request processing stopped: curl multi poll failed, "
                    "errorCode: "
                    << static_cast<int>(code)
                    << ", error: "
                    << curl_multi_strerror(static_cast<CURLMcode>(code)));
                break;
            }
        }
    }while(runningHandles > 0);
}

void LibCurlMultiHandle::stop()
{
    LIBCURL_LOG_INFO(
        "LibCurlMultiHandle shutdown started, address: " << this);

    mDataPrivate->stop();

    LIBCURL_LOG_INFO(
        "LibCurlMultiHandle shutdown finished, address: " << this);
}

bool LibCurlMultiHandle::cancelRequest(const std::string& requestId)
{
    return mDataPrivate->cancelRequest(requestId);
}

bool LibCurlMultiHandle::DataPrivate::cancelRequest(const std::string& requestId)
{
    std::shared_ptr<LibCurlEasyHandle> handleToCancel;
    size_t totalCount = 0;
    bool adding = false;
    {
        std::scoped_lock lo(mRequestsAccess);
        auto iter = std::find_if(mRequests.begin(), mRequests.end(),
            [&requestId](const auto& request) {
                return request->getRequestId() == requestId;
            });
        
        if (iter == mRequests.end()) {
            LIBCURL_LOG_WARN(
                "Network request cancellation failed in multi handle: "
                "request was not found, requestId: "
                << requestId);
            return false;
        }
        
        handleToCancel = *iter;
        adding = mAddingRequests.contains(handleToCancel.get());
        mRequests.erase(iter);
        totalCount = mRequests.size();
        if (!adding)
        {
            mCanceledRequests.push_back(handleToCancel);
        }
    }

    int running = mLastRunningCount.load(std::memory_order_acquire);
    int queued = (std::max)(0, static_cast<int>(totalCount) - running);
    LIBCURL_LOG_INFO(
        "Network request canceled in multi handle, requestId: "
        << requestId
        << ", totalCount: "
        << totalCount
        << ", runningCount: "
        << running
        << ", queuedCount: "
        << queued);

    // A request still being added is finalized by addEasyHandle. For an active
    // request, drainCanceledRequests safely defers removal if curl is currently
    // invoking this thread's header/body callback.
    if (!adding)
    {
        drainCanceledRequests();
    }
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start LibCurlMultiHandle Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
