#include "OSUtils_Mac.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <unistd.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <CoreFoundation/CoreFoundation.h>

namespace ucf::utilities{
std::string OSUtils_Mac::getOSVersion()
{
    char str[256];
    size_t size = sizeof(str);
    if (sysctlbyname("kern.osrelease", str, &size, NULL, 0) == 0) 
    {
        return std::string(str);
    } 
    else 
    {
        return "Unknown Apple OS version";
    }
}

std::string OSUtils_Mac::getCPUInfo()
{
    char buffer[256];
    size_t size = sizeof(buffer);
    if (sysctlbyname("machdep.cpu.brand_string", buffer, &size, nullptr, 0) == 0)
        return std::string(buffer);
    return "Unknown CPU";
}

std::string OSUtils_Mac::getSystemLanguage()
{
    CFArrayRef langs = CFLocaleCopyPreferredLanguages();
    if (langs && CFArrayGetCount(langs) > 0) {
        CFStringRef lang = (CFStringRef)CFArrayGetValueAtIndex(langs, 0);
        char buffer[256];
        if (CFStringGetCString(lang, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
            CFRelease(langs);
            return std::string(buffer);
        }
        CFRelease(langs);
    }
    return "en-US";
}

MemoryInfo OSUtils_Mac::getMemoryInfo() {
    MemoryInfo info = {0, 0};

    int64_t memSize = 0;
    size_t size = sizeof(memSize);
    if (sysctlbyname("hw.memsize", &memSize, &size, nullptr, 0) == 0) {
        info.totalMemoryBytes = memSize;
    }

    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;
    if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) == KERN_SUCCESS) {
        uint64_t pageSize = sysconf(_SC_PAGESIZE);
        info.availableMemoryBytes = (vmstat.free_count + vmstat.inactive_count) * pageSize;
    }

    return info;
}

std::string OSUtils_Mac::getGPUInfo()
{
    CFMutableDictionaryRef matchDict = IOServiceMatching("IOPCIDevice");
    io_iterator_t iter;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, matchDict, &iter) != KERN_SUCCESS)
        return "";

    std::string result;
    io_object_t obj;
    while ((obj = IOIteratorNext(iter)) != 0) {
        CFMutableDictionaryRef properties = nullptr;
        if (IORegistryEntryCreateCFProperties(obj, &properties, kCFAllocatorDefault, 0) == KERN_SUCCESS) {
            CFTypeRef model = CFDictionaryGetValue(properties, CFSTR("model"));
            if (model && CFGetTypeID(model) == CFDataGetTypeID()) {
                CFDataRef modelData = (CFDataRef)model;
                const UInt8* bytes = CFDataGetBytePtr(modelData);
                CFIndex length = CFDataGetLength(modelData);
                std::string modelName(reinterpret_cast<const char*>(bytes), length);
                result += "GPU: " + modelName + "\n";
            }
            CFRelease(properties);
        }
        IOObjectRelease(obj);
    }
    IOObjectRelease(iter);
    return result;
}
}