#if defined(_WIN32)

#include "WindowsMemoryMonitor.h"

#include <windows.h>
#include <psapi.h>

namespace ucf::service {

MemoryInfo WindowsMemoryMonitor::getMemoryUsage() const
{
    MemoryInfo info;
    
    // Get process memory info
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), 
                             reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), 
                             sizeof(pmc)))
    {
        info.physicalBytes = pmc.WorkingSetSize;
        info.virtualBytes = pmc.PrivateUsage;
        info.peakPhysicalBytes = pmc.PeakWorkingSetSize;
    }
    
    // Get available system memory
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus))
    {
        info.availableSystemBytes = memStatus.ullAvailPhys;
    }
    
    return info;
}

} // namespace ucf::service

#endif // defined(_WIN32)
