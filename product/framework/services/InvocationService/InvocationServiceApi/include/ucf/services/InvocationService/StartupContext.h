#pragma once

#include <string>
#include <vector>

namespace ucf::service{

struct StartupContext final
{
    std::vector<std::string> commandLineArguments;
};

}
