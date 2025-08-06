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

std::string OSUtils_Linux::getCPUInfo()
{
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            auto pos = line.find(":");
            if (pos != std::string::npos)
                return line.substr(pos + 2);
        }
    }
    return "Unknown CPU";
}

std::string OSUtils_Linux::getSystemLanguage()
{
    const char* lang = getenv("LANG");
    if (lang) {
        return std::string(lang);
    }
    return "en-US";
}

std::string OSUtils_Linux::execCommand(const std::string& cmd)
{
    std::array<char, 128> buffer;
    std::string result;

    // 用 std::string 参数调用 popen，需要调用 c_str()
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string OSUtils_Linux::getGPUInfo()
{
    // 查询所有 VGA、3D、2D 控制器设备
    std::string output = execCommand("lspci | grep -i 'vga\\|3d\\|2d'");
    return output.empty() ? "" : output;
}
} // namespace ucf::utilities