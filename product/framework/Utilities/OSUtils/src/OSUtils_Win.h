#pragma once

#include <string>
namespace ucf::utilities{
class OSUtils_Win final
{
public:
    static std::string getOSVersion();
};
}