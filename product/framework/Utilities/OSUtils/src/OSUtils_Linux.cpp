#include "OSUtils_Linux.h"
#include <fstream>

namespace ucf::utilities{
std::string OSUtils_Linux::getOSVersion()
{
    std::ifstream file("/etc/os-release");
    std::string line, version;
    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            if (line.find("VERSION=") == 0)
            {
                version = line.substr(8);
                if (version.front() == '"' && version.back() == '"')
                {
                    version = version.substr(1, version.size() - 2);
                }
                break;
            }
        }
        file.close();
    }
    return !version.empty() ? version : "Unknown Linux version";
}
}