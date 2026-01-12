#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>


#include "LibCurlMultiHandle.h"
#include "LibCurlEasyHandle.h"
#include "LibCurlMultiHandleManager.h"

#include "LibCurlClientLogger.h"

namespace ucf::utilities::network::libcurl{
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
private:
    std::unique_ptr<LibCurlMultiHandle> mMultiHandle;
    std::atomic_bool mStop;
    std::thread mLoopThread;
    std::condition_variable mRequestCV;
    std::mutex mRequestCVMutex;
    std::atomic_bool mHasReuqests;
};

LibCurlMultiHandleManager::DataPrivate::DataPrivate()
    : mMultiHandle(std::make_unique<LibCurlMultiHandle>())
    , mStop(false)
    , mHasReuqests(false)
{

}

LibCurlMultiHandleManager::DataPrivate::~DataPrivate()
{
    LIBCURL_LOG_DEBUG("delete LibCurlMultiHandleManager::DataPrivate");
    stopLoop();
    LIBCURL_LOG_DEBUG("delete LibCurlMultiHandleManager::DataPrivate done");
}

void LibCurlMultiHandleManager::DataPrivate::insertRequest(std::shared_ptr<LibCurlEasyHandle> request)
{
    mMultiHandle->addEasyHandle(request);
    {
        std::scoped_lock lo(mRequestCVMutex);
        mHasReuqests.store(true, std::memory_order_release);
    }
    mRequestCV.notify_one();
}

void LibCurlMultiHandleManager::DataPrivate::runLoop()
{
    mLoopThread = std::thread([this](){
        while(!mStop.load(std::memory_order_acquire))
        {
            {
                std::unique_lock lo(mRequestCVMutex);
                mRequestCV.wait(lo,[this](){
                    return mHasReuqests.load(std::memory_order_acquire) || mStop.load(std::memory_order_acquire);
                });

                mHasReuqests.store(false, std::memory_order_release);
            }
            if (mStop.load(std::memory_order_acquire))
            {
                LIBCURL_LOG_DEBUG("already stopped, won't perform requests any more");
                break;
            }
            mMultiHandle->performRequests();
        }
    });
}

void LibCurlMultiHandleManager::DataPrivate::stopLoop()
{
    mStop.store(true, std::memory_order_release);
    mMultiHandle->stop();
    if (mLoopThread.joinable())
    {
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
    LIBCURL_LOG_DEBUG("created, address:" << this);
}

LibCurlMultiHandleManager::~LibCurlMultiHandleManager()
{
    LIBCURL_LOG_DEBUG("destoryed, address:" << this);
}

void LibCurlMultiHandleManager::runLoop()
{
    LIBCURL_LOG_DEBUG("start looping, address:" << this);
    mDataPrivate->runLoop();
    LIBCURL_LOG_DEBUG("start looping done, address:" << this);
}

void LibCurlMultiHandleManager::stopLoop()
{
    LIBCURL_LOG_DEBUG("start stopping loop, address:" << this);
    mDataPrivate->stopLoop();
    LIBCURL_LOG_DEBUG("finish stopping loop, address:" << this);
}

void LibCurlMultiHandleManager::insert(std::shared_ptr<LibCurlEasyHandle> request)
{
    mDataPrivate->insertRequest(request);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish LibCurlMultiHandleManager Logic////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
