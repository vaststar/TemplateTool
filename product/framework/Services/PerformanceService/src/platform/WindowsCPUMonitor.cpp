#include "WindowsCPUMonitor.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace ucf::service {

WindowsCPUMonitor::WindowsCPUMonitor(std::chrono::milliseconds sampleInterval)
    : mSampleInterval(sampleInterval)
{
}

WindowsCPUMonitor::~WindowsCPUMonitor()
{
    stop();
}

void WindowsCPUMonitor::start()
{
    if (mRunning.exchange(true))
    {
        return;
    }
    
    mSampleThread = std::thread(&WindowsCPUMonitor::sampleLoop, this);
}

void WindowsCPUMonitor::stop()
{
    if (!mRunning.exchange(false))
    {
        return;
    }
    
    if (mSampleThread.joinable())
    {
        mSampleThread.join();
    }
}

double WindowsCPUMonitor::getCPUUsage() const
{
    return mCPUUsage.load();
}

unsigned int WindowsCPUMonitor::getCPUCoreCount() const
{
    return std::thread::hardware_concurrency();
}

void WindowsCPUMonitor::sampleLoop()
{
    uint64_t prevCPUTime = getProcessCPUTime();
    auto prevWallTime = std::chrono::steady_clock::now();
    
    while (mRunning.load())
    {
        std::this_thread::sleep_for(mSampleInterval);
        
        if (!mRunning.load())
        {
            break;
        }
        
        uint64_t currCPUTime = getProcessCPUTime();
        auto currWallTime = std::chrono::steady_clock::now();
        
        uint64_t cpuTimeDelta = currCPUTime - prevCPUTime;
        auto wallTimeDelta = std::chrono::duration_cast<std::chrono::microseconds>(
            currWallTime - prevWallTime).count();
        
        if (wallTimeDelta > 0)
        {
            // cpuTimeDelta is in 100-nanosecond intervals, convert to microseconds
            double cpuTimeMicros = static_cast<double>(cpuTimeDelta) / 10.0;
            double usage = (cpuTimeMicros / static_cast<double>(wallTimeDelta)) * 100.0;
            
            mCPUUsage.store(usage);
        }
        
        prevCPUTime = currCPUTime;
        prevWallTime = currWallTime;
    }
}

uint64_t WindowsCPUMonitor::getProcessCPUTime() const
{
#ifdef _WIN32
    FILETIME createTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime))
    {
        // FILETIME is in 100-nanosecond intervals
        ULARGE_INTEGER kernel, user;
        kernel.LowPart = kernelTime.dwLowDateTime;
        kernel.HighPart = kernelTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;
        
        return kernel.QuadPart + user.QuadPart;
    }
#endif
    return 0;
}

} // namespace ucf::service
