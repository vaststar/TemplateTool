#pragma once

#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "LibCurlMultiHandle.h"
#include "LibCurlEasyHandle.h"

namespace ucf::network::libcurl{
class LibCurlMultiHandleManager final
{
public:
    LibCurlMultiHandleManager();
    ~LibCurlMultiHandleManager();
    LibCurlMultiHandleManager(const LibCurlMultiHandleManager&) = delete;
    LibCurlMultiHandleManager(LibCurlMultiHandleManager&&) = delete;
    LibCurlMultiHandleManager& operator=(const LibCurlMultiHandleManager&) = delete;
    LibCurlMultiHandleManager& operator=(LibCurlMultiHandleManager&&) = delete;
public:
    void runLoop();
    void stopLoop();
    void insert(std::shared_ptr<LibCurlEasyHandle> request);
private:
    std::unique_ptr<LibCurlMultiHandle> mMultiHandle;
    std::mutex mStopMutex;
    std::condition_variable mCondition;
    std::atomic_bool mStop;
    std::thread mLoopThread;
};
}