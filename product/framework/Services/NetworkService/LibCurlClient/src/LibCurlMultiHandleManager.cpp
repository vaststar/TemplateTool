#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>


#include "LibCurlMultiHandle.h"
#include "LibCurlEasyHandle.h"
#include "LibCurlMultiHandleManager.h"

#include "LibCurlClientLogger.h"

namespace ucf::service::network::libcurl{
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
    std::mutex mStopMutex;
    std::condition_variable mCondition;
    std::atomic_bool mStop;
    std::thread mLoopThread;
};

LibCurlMultiHandleManager::DataPrivate::DataPrivate()
    : mMultiHandle(std::make_unique<LibCurlMultiHandle>())
    , mStop(false)
{

}

LibCurlMultiHandleManager::DataPrivate::~DataPrivate()
{
    LIBCURL_LOG_DEBUG("delete LibCurlMultiHandleManager::DataPrivate");
    {
        std::scoped_lock lo(mStopMutex);
        mStop = true;
    }
    
    if (mLoopThread.joinable())
    {
        mLoopThread.join();
    }
    LIBCURL_LOG_DEBUG("delete LibCurlMultiHandleManager::DataPrivate done");
}

void LibCurlMultiHandleManager::DataPrivate::insertRequest(std::shared_ptr<LibCurlEasyHandle> request)
{
    mMultiHandle->addEasyHandle(request);
}

void LibCurlMultiHandleManager::DataPrivate::runLoop()
{
    mLoopThread = std::thread([this](){
        while(true)
        {
            mMultiHandle->performRequests();
            std::unique_lock<std::mutex> lo(mStopMutex);
            if (mStop)
            {
                break;
            }
        }
        mCondition.notify_all();
    });
}

void LibCurlMultiHandleManager::DataPrivate::stopLoop()
{
    std::unique_lock<std::mutex> lo(mStopMutex);
    mStop = true;
    mCondition.wait(lo);
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
