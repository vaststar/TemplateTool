#pragma once

#include <string>
#include <ucf/Utilities/OSUtils/OSUtils.h>
namespace ucf::utilities{
class OSUtils_Mac final
{
public:
    static std::string getOSVersion();
    static std::string getCPUInfo();
    static std::string getSystemLanguage();
    static MemoryInfo getMemoryInfo();
    static std::string getGPUInfo();
};
}