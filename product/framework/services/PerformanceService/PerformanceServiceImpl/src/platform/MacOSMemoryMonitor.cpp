#if defined(__APPLE__)

#include "MacOSMemoryMonitor.h"

#include <mach/mach.h>
#include <mach/task.h>
#include <mach/mach_init.h>
#include <sys/sysctl.h>

namespace ucf::service {

MemoryInfo MacOSMemoryMonitor::getMemoryUsage() const
{
    MemoryInfo info;
    
    // Get process memory info
    mach_task_basic_info_data_t taskInfo;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, 
                  reinterpret_cast<task_info_t>(&taskInfo), &infoCount) == KERN_SUCCESS)
    {
        info.physicalBytes = taskInfo.resident_size;
        info.virtualBytes = taskInfo.virtual_size;
    }
    
    // Get peak memory (resident_size_max)
    task_vm_info_data_t vmInfo;
    mach_msg_type_number_t vmInfoCount = TASK_VM_INFO_COUNT;
    
    if (task_info(mach_task_self(), TASK_VM_INFO,
                  reinterpret_cast<task_info_t>(&vmInfo), &vmInfoCount) == KERN_SUCCESS)
    {
        info.peakPhysicalBytes = vmInfo.resident_size_peak;
    }
    
    // Get available system memory
    vm_statistics64_data_t vmStats;
    mach_msg_type_number_t vmStatsCount = HOST_VM_INFO64_COUNT;
    
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64,
                          reinterpret_cast<host_info64_t>(&vmStats), &vmStatsCount) == KERN_SUCCESS)
    {
        vm_size_t pageSize;
        host_page_size(mach_host_self(), &pageSize);
        info.availableSystemBytes = vmStats.free_count * pageSize;
    }
    
    return info;
}

} // namespace ucf::service

#endif // defined(__APPLE__)
