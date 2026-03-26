#include "LinuxCPUMonitor.h"

#include <fstream>
#include <sstream>
#include <unistd.h>

namespace ucf::service {

LinuxCPUMonitor::LinuxCPUMonitor(std::chrono::milliseconds sampleInterval)
    : mSampleInterval(sampleInterval)
    , mClockTicksPerSecond(sysconf(_SC_CLK_TCK))
{
}

LinuxCPUMonitor::~LinuxCPUMonitor()
{
    stop();
}

void LinuxCPUMonitor::start()
{
    if (mRunning.exchange(true))
    {
        return;
    }
    
    mSampleThread = std::thread(&LinuxCPUMonitor::sampleLoop, this);
}

void LinuxCPUMonitor::stop()
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

double LinuxCPUMonitor::getCPUUsage() const
{
    return mCPUUsage.load();
}

unsigned int LinuxCPUMonitor::getCPUCoreCount() const
{
    return std::thread::hardware_concurrency();
}

void LinuxCPUMonitor::sampleLoop()
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
        auto wallTimeDelta = std::chrono::duration_cast<std::chrono::milliseconds>(
            currWallTime - prevWallTime).count();
        
        if (wallTimeDelta > 0 && mClockTicksPerSecond > 0)
        {
            // Convert clock ticks to milliseconds
            double cpuTimeMs = (static_cast<double>(cpuTimeDelta) * 1000.0) / 
                               static_cast<double>(mClockTicksPerSecond);
            double usage = (cpuTimeMs / static_cast<double>(wallTimeDelta)) * 100.0;
            
            mCPUUsage.store(usage);
        }
        
        prevCPUTime = currCPUTime;
        prevWallTime = currWallTime;
    }
}

uint64_t LinuxCPUMonitor::getProcessCPUTime() const
{
    // Read from /proc/self/stat
    std::ifstream stat("/proc/self/stat");
    if (!stat.is_open())
    {
        return 0;
    }
    
    std::string line;
    std::getline(stat, line);
    
    // Skip past the comm field (which may contain spaces in parentheses)
    size_t start = line.rfind(')');
    if (start == std::string::npos)
    {
        return 0;
    }
    
    std::istringstream iss(line.substr(start + 2));
    
    // Fields after (comm): state, ppid, pgrp, session, tty_nr, tpgid, flags,
    // minflt, cminflt, majflt, cmajflt, utime(14), stime(15), cutime, cstime
    std::string state;
    long ppid, pgrp, session, tty_nr, tpgid;
    unsigned long flags, minflt, cminflt, majflt, cmajflt;
    unsigned long utime, stime;
    
    iss >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags
        >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime;
    
    return utime + stime;
}

} // namespace ucf::service
