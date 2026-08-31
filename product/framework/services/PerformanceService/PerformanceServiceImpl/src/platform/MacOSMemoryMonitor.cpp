#if defined(__APPLE__)

#include "MacOSMemoryMonitor.h"

#include <mach/mach.h>
#include <mach/task.h>
#include <mach/mach_init.h>

#include <limits>

namespace ucf::service {
namespace {

std::optional<uint64_t> pageCountToBytes(uint64_t pageCount, uint64_t pageSize)
{
    if (pageSize == 0 || pageCount > std::numeric_limits<uint64_t>::max() / pageSize)
    {
        return std::nullopt;
    }
    return pageCount * pageSize;
}

} // namespace

MemoryInfo MacOSMemoryMonitor::getMemoryUsage() const
{
    MemoryInfo info;

    task_vm_info_data_t vmInfo{};
    mach_msg_type_number_t vmInfoCount = TASK_VM_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_VM_INFO,
                  reinterpret_cast<task_info_t>(&vmInfo), &vmInfoCount) == KERN_SUCCESS)
    {
        info.processResidentBytes = vmInfo.resident_size;
        info.processPeakResidentBytes = vmInfo.resident_size_peak;
        info.processVirtualAddressSpaceBytes = vmInfo.virtual_size;
    }

    vm_statistics64_data_t vmStats{};
    mach_msg_type_number_t vmStatsCount = HOST_VM_INFO64_COUNT;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64,
                          reinterpret_cast<host_info64_t>(&vmStats), &vmStatsCount) == KERN_SUCCESS)
    {
        vm_size_t pageSize{0};
        if (host_page_size(mach_host_self(), &pageSize) == KERN_SUCCESS)
        {
            // Inactive pages can be reclaimed without swapping. Speculative
            // pages are already included in free_count and must not be added.
            const uint64_t availablePageCount =
                static_cast<uint64_t>(vmStats.free_count) + vmStats.inactive_count;
            info.systemAvailablePhysicalBytes = pageCountToBytes(availablePageCount, pageSize);
        }
    }

    return info;
}

} // namespace ucf::service

#endif // defined(__APPLE__)
