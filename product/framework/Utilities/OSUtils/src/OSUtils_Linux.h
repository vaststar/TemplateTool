#pragma once

#include <string>
namespace ucf::utilities{
class OSUtils_Linux final
{
public:
    static std::string getOSVersion();
};
}