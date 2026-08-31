#if defined(__linux__)

#include "LinuxMemoryMonitor.h"

#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>

namespace ucf::service {
namespace {

std::optional<uint64_t> parseKibibytes(const std::string& line, const char* expectedKey)
{
    std::istringstream stream(line);
    std::string key;
    std::string unit;
    uint64_t kibibytes{0};
    if (!(stream >> key >> kibibytes >> unit) || key != expectedKey || unit != "kB" ||
        kibibytes > std::numeric_limits<uint64_t>::max() / 1024ULL)
    {
        return std::nullopt;
    }
    return kibibytes * 1024ULL;
}

} // namespace

MemoryInfo LinuxMemoryMonitor::getMemoryUsage() const
{
    MemoryInfo info;

    std::ifstream status("/proc/self/status");
    if (status.is_open())
    {
        std::string line;
        while (std::getline(status, line))
        {
            if (line.starts_with("VmRSS:"))
            {
                info.processResidentBytes = parseKibibytes(line, "VmRSS:");
            }
            else if (line.starts_with("VmHWM:"))
            {
                info.processPeakResidentBytes = parseKibibytes(line, "VmHWM:");
            }
            else if (line.starts_with("VmSize:"))
            {
                info.processVirtualAddressSpaceBytes = parseKibibytes(line, "VmSize:");
            }
        }
    }

    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open())
    {
        std::string line;
        while (std::getline(meminfo, line))
        {
            if (line.starts_with("MemAvailable:"))
            {
                info.systemAvailablePhysicalBytes = parseKibibytes(line, "MemAvailable:");
                break;
            }
        }
    }

    return info;
}

} // namespace ucf::service

#endif // defined(__linux__)
