#include <thread>

#include "LibCurlMultiHandleManager.h"

#include "LibCurlClientLogger.h"

namespace ucf::network::libcurl{
LibCurlMultiHandleManager::LibCurlMultiHandleManager()
    : mMultiHandle(std::make_unique<LibCurlMultiHandle>())
    , mStop(false)
{
    LIBCURL_LOG_DEBUG("created, address:" << this);
}

LibCurlMultiHandleManager::~LibCurlMultiHandleManager()
{
    LIBCURL_LOG_DEBUG("destoryed, address:" << this);
    
    {
        std::scoped_lock lo(mStopMutex);
        mStop = true;
    }
    if (mLoopThread.joinable())
    {
        mLoopThread.join();
    }
}

void LibCurlMultiHandleManager::runLoop()
{
    LIBCURL_LOG_DEBUG("start looping, address:" << this);
    mLoopThread = std::thread([this](){
        do
        {
            int runningHandles = 0;
            do
            {
                if (CURLM_OK !=  mMultiHandle->perform(&runningHandles))
                {
                    break;
                }

                if (runningHandles > 0)
                {
                    if (CURLM_OK !=  mMultiHandle->poll(nullptr, 0, 100, nullptr))
                    {
                        break;
                    }
                }
                //readAll();
            }while(runningHandles > 0);

            std::unique_lock<std::mutex> lo(mStopMutex);
            if (mStop)
            {
                break;
            }
        }while(true);
        mCondition.notify_all();
    });
}

void LibCurlMultiHandleManager::stopLoop()
{
    LIBCURL_LOG_DEBUG("start stopping loop, address:" << this);
    std::unique_lock<std::mutex> lo(mStopMutex);
    mStop = true;
    mCondition.wait(lo);
    LIBCURL_LOG_DEBUG("finish stopping loop");
}

}
