#if defined(__linux__)

#include "LinuxMemoryMonitor.h"

#include <fstream>
#include <string>
#include <unistd.h>

namespace ucf::service {

MemoryInfo LinuxMemoryMonitor::getMemoryUsage() const
{
    MemoryInfo info;
    
    // Read from /proc/self/statm
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open())
    {
        long pageSize = sysconf(_SC_PAGESIZE);
        long vmPages = 0, rssPages = 0;
        
        statm >> vmPages >> rssPages;
        
        info.virtualBytes = vmPages * pageSize;
        info.physicalBytes = rssPages * pageSize;
    }
    
    // Read peak from /proc/self/status
    std::ifstream status("/proc/self/status");
    if (status.is_open())
    {
        std::string line;
        while (std::getline(status, line))
        {
            if (line.find("VmPeak:") == 0)
            {
                // Format: "VmPeak:    12345 kB"
                size_t kb = 0;
                sscanf(line.c_str(), "VmPeak: %zu kB", &kb);
                info.peakPhysicalBytes = kb * 1024;
            }
        }
    }
    
    // Read available system memory from /proc/meminfo
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open())
    {
        std::string line;
        while (std::getline(meminfo, line))
        {
            if (line.find("MemAvailable:") == 0)
            {
                size_t kb = 0;
                sscanf(line.c_str(), "MemAvailable: %zu kB", &kb);
                info.availableSystemBytes = kb * 1024;
                break;
            }
        }
    }
    
    return info;
}

} // namespace ucf::service

#endif // defined(__linux__)
