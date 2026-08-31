#if defined(_WIN32)

#include "WindowsMemoryMonitor.h"

#include <windows.h>
#include <psapi.h>

namespace ucf::service {

MemoryInfo WindowsMemoryMonitor::getMemoryUsage() const
{
    MemoryInfo info;

    PROCESS_MEMORY_COUNTERS_EX pmc{};
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), 
                             reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), 
                             sizeof(pmc)))
    {
        info.processResidentBytes = pmc.WorkingSetSize;
        info.processPeakResidentBytes = pmc.PeakWorkingSetSize;
        info.processPrivateCommittedBytes = pmc.PrivateUsage;
    }

    MEMORYSTATUSEX memStatus{};
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus))
    {
        info.systemAvailablePhysicalBytes = memStatus.ullAvailPhys;
    }

    return info;
}

} // namespace ucf::service

#endif // defined(_WIN32)
