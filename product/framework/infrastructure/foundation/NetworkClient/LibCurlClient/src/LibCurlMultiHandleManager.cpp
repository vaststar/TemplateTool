#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <string>

#include <curl/curl.h>

#include "LibCurlMultiHandle.h"
#include "LibCurlEasyHandle.h"
#include "LibCurlMultiHandleManager.h"

#include "LibCurlClientLogger.h"

namespace ucf::infrastructure::network::libcurl{

namespace {

class CurlGlobalRuntime final
{
public:
    CurlGlobalRuntime()
    {
        LIBCURL_LOG_DEBUG("libcurl global initialization started");

        const auto result = curl_global_init(CURL_GLOBAL_ALL);
        if (result != CURLE_OK)
        {
            LIBCURL_LOG_ERROR(
                "libcurl global initialization failed, errorCode: "
                << static_cast<int>(result)
                << ", error: "
                << curl_easy_strerror(result));

            throw std::runtime_error(
                std::string("libcurl global initialization failed: ")
                + curl_easy_strerror(result));
        }

        LIBCURL_LOG_DEBUG("libcurl global initialization finished");
    }

    ~CurlGlobalRuntime()
    {
        LIBCURL_LOG_DEBUG("libcurl global cleanup started");

        curl_global_cleanup();

        LIBCURL_LOG_DEBUG("libcurl global cleanup finished");
    }

    CurlGlobalRuntime(const CurlGlobalRuntime&) = delete;
    CurlGlobalRuntime(CurlGlobalRuntime&&) = delete;
    CurlGlobalRuntime& operator=(const CurlGlobalRuntime&) = delete;
    CurlGlobalRuntime& operator=(CurlGlobalRuntime&&) = delete;
};

}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class LibCurlMultiHandleManager::DataPrivate
{
public:
    DataPrivate();
    ~DataPrivate();
    void insertRequest(std::shared_ptr<LibCurlEasyHandle> request);
    void runLoop();
    void stopLoop();
    bool cancelRequest(const std::string& requestId);
private:
    CurlGlobalRuntime mCurlGlobalRuntime;
    std::unique_ptr<LibCurlMultiHandle> mMultiHandle;
    std::atomic_bool mStop;
    std::thread mLoopThread;
    std::condition_variable mRequestCV;
    std::mutex mRequestCVMutex;
    std::atomic_bool mHasRequests;
};

LibCurlMultiHandleManager::DataPrivate::DataPrivate()
    : mCurlGlobalRuntime()
    , mMultiHandle(std::make_unique<LibCurlMultiHandle>())
    , mStop(false)
    , mHasRequests(false)
{

}

LibCurlMultiHandleManager::DataPrivate::~DataPrivate()
{
    LIBCURL_LOG_DEBUG(
        "LibCurlMultiHandleManager DataPrivate shutdown started, address: "
        << this);

    stopLoop();

    LIBCURL_LOG_DEBUG(
        "LibCurlMultiHandleManager DataPrivate shutdown finished, address: "
        << this);
}

void LibCurlMultiHandleManager::DataPrivate::insertRequest(std::shared_ptr<LibCurlEasyHandle> request)
{
    mMultiHandle->addEasyHandle(request);
    {
        std::scoped_lock lo(mRequestCVMutex);
        mHasRequests.store(true, std::memory_order_release);
    }
    mRequestCV.notify_one();
}

void LibCurlMultiHandleManager::DataPrivate::runLoop()
{
    mLoopThread = std::thread([this](){
        LIBCURL_LOG_DEBUG(
            "LibCurlMultiHandleManager worker loop started, address: " << this);

        while(!mStop.load(std::memory_order_acquire))
        {
            {
                std::unique_lock lo(mRequestCVMutex);
                mRequestCV.wait(lo,[this](){
                    return mHasRequests.load(std::memory_order_acquire) || mStop.load(std::memory_order_acquire);
                });

                mHasRequests.store(false, std::memory_order_release);
            }
            if (mStop.load(std::memory_order_acquire))
            {
                LIBCURL_LOG_DEBUG(
                    "LibCurlMultiHandleManager request processing stopped: "
                    "shutdown was requested, address: "
                    << this);
                break;
            }
            mMultiHandle->performRequests();
        }

        LIBCURL_LOG_DEBUG(
            "LibCurlMultiHandleManager worker loop finished, address: " << this);
    });
}

void LibCurlMultiHandleManager::DataPrivate::stopLoop()
{
    mStop.store(true, std::memory_order_release);
    mMultiHandle->stop();
    if (mLoopThread.joinable())
    {
        mRequestCV.notify_one();
        mLoopThread.join();
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start LibCurlMultiHandleManager Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
LibCurlMultiHandleManager::LibCurlMultiHandleManager()
    : mDataPrivate(std::make_unique<LibCurlMultiHandleManager::DataPrivate>())
{
    LIBCURL_LOG_DEBUG("LibCurlMultiHandleManager constructed, address: " << this);
}

LibCurlMultiHandleManager::~LibCurlMultiHandleManager()
{
    LIBCURL_LOG_DEBUG("LibCurlMultiHandleManager destroying, address: " << this);
}

void LibCurlMultiHandleManager::runLoop()
{
    LIBCURL_LOG_DEBUG(
        "LibCurlMultiHandleManager worker startup started, address: " << this);

    mDataPrivate->runLoop();

    LIBCURL_LOG_DEBUG(
        "LibCurlMultiHandleManager worker startup finished, address: " << this);
}

void LibCurlMultiHandleManager::stopLoop()
{
    LIBCURL_LOG_DEBUG(
        "LibCurlMultiHandleManager worker shutdown started, address: " << this);

    mDataPrivate->stopLoop();

    LIBCURL_LOG_DEBUG(
        "LibCurlMultiHandleManager worker shutdown finished, address: " << this);
}

void LibCurlMultiHandleManager::insert(std::shared_ptr<LibCurlEasyHandle> request)
{
    mDataPrivate->insertRequest(request);
}

bool LibCurlMultiHandleManager::cancelRequest(const std::string& requestId)
{
    return mDataPrivate->cancelRequest(requestId);
}

bool LibCurlMultiHandleManager::DataPrivate::cancelRequest(const std::string& requestId)
{
    return mMultiHandle->cancelRequest(requestId);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish LibCurlMultiHandleManager Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
