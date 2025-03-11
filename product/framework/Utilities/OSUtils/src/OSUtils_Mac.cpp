#include "OSUtils_Mac.h"
#include <sys/types.h>
#include <sys/sysctl.h>

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
}