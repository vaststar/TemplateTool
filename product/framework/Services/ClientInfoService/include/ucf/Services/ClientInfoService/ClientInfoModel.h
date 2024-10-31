#pragma once

#include <string>

namespace ucf::service::model{

struct Version
{
    std::string majorVersion;
    std::string minorVersion;
    std::string patchVersion;
};

enum class OSType
{
    Windows32,
    Windows64,
    OSX,
    Linux
};
}