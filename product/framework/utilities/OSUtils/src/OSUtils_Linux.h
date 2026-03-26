#pragma once

#include <string>
#include <ucf/Utilities/OSUtils/OSUtils.h>
namespace ucf::utilities{
class OSUtils_Linux final
{
public:
    static std::string getOSVersion();
    static std::string getCPUInfo();
    static std::string getSystemLanguage();
    static std::string getGPUInfo();
    static MemoryInfo getMemoryInfo();
private:
    static std::string execCommand(const std::string& cmd);
};
}