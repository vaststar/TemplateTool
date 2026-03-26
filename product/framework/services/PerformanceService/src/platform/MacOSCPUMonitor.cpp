#include "MacOSCPUMonitor.h"

#include <mach/mach.h>
#include <mach/task_info.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace ucf::service {

MacOSCPUMonitor::MacOSCPUMonitor(std::chrono::milliseconds sampleInterval)
    : mSampleInterval(sampleInterval)
{
}

MacOSCPUMonitor::~MacOSCPUMonitor()
{
    stop();
}

void MacOSCPUMonitor::start()
{
    if (mRunning.exchange(true))
    {
        return; // Already running
    }
    
    mSampleThread = std::thread(&MacOSCPUMonitor::sampleLoop, this);
}

void MacOSCPUMonitor::stop()
{
    if (!mRunning.exchange(false))
    {
        return; // Not running
    }
    
    if (mSampleThread.joinable())
    {
        mSampleThread.join();
    }
}

double MacOSCPUMonitor::getCPUUsage() const
{
    return mCPUUsage.load();
}

unsigned int MacOSCPUMonitor::getCPUCoreCount() const
{
    return std::thread::hardware_concurrency();
}

void MacOSCPUMonitor::sampleLoop()
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
        
        // Calculate CPU usage
        uint64_t cpuTimeDelta = currCPUTime - prevCPUTime;
        auto wallTimeDelta = std::chrono::duration_cast<std::chrono::microseconds>(
            currWallTime - prevWallTime).count();
        
        if (wallTimeDelta > 0)
        {
            // CPU usage as percentage (can exceed 100% on multi-core)
            double usage = (static_cast<double>(cpuTimeDelta) / static_cast<double>(wallTimeDelta)) * 100.0;
            
            // Normalize by core count for 0-100% range
            // Uncomment if you want per-core normalization:
            // usage /= getCPUCoreCount();
            
            mCPUUsage.store(usage);
        }
        
        prevCPUTime = currCPUTime;
        prevWallTime = currWallTime;
    }
}

uint64_t MacOSCPUMonitor::getProcessCPUTime() const
{
    task_thread_times_info_data_t timeInfo;
    mach_msg_type_number_t count = TASK_THREAD_TIMES_INFO_COUNT;
    
    kern_return_t kr = task_info(mach_task_self(),
                                  TASK_THREAD_TIMES_INFO,
                                  reinterpret_cast<task_info_t>(&timeInfo),
                                  &count);
    
    if (kr != KERN_SUCCESS)
    {
        return 0;
    }
    
    // Convert to microseconds
    uint64_t userTime = static_cast<uint64_t>(timeInfo.user_time.seconds) * 1000000 +
                        static_cast<uint64_t>(timeInfo.user_time.microseconds);
    uint64_t systemTime = static_cast<uint64_t>(timeInfo.system_time.seconds) * 1000000 +
                          static_cast<uint64_t>(timeInfo.system_time.microseconds);
    
    return userTime + systemTime;
}

} // namespace ucf::service
