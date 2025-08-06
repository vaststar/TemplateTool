#pragma once

#include <string>
namespace ucf::utilities{
class OSUtils_Linux final
{
public:
    static std::string getOSVersion();
    static std::string getCPUInfo();
    static std::string getSystemLanguage();
    static std::string getGPUInfo();
private:
    static std::string execCommand(const std::string& cmd);
};
}