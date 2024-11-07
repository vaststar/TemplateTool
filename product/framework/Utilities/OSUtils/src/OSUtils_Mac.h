#pragma once

#include <string>
namespace ucf::utilities{
class OSUtils_Mac final
{
public:
    static std::string getOSVersion();
};
}