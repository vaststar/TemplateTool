#pragma once

#include <string>
#include <ucf/Utilities/OSUtils/OSUtils.h>
namespace ucf::utilities{
class OSUtils_Win final
{
public:
    static std::string getOSVersion();
    static std::string getCPUInfo();
    static MemoryInfo getMemoryInfo();
    static std::string getSystemLanguage();
    static std::string getGPUInfo();
};
}