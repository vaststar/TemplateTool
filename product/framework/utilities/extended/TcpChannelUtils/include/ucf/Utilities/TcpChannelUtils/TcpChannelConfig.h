#pragma once

#include <string>
#include <ucf/Utilities/TcpChannelUtils/TcpChannelUtilsExport.h>

namespace ucf::utilities {

struct TcpChannelConfig
{
    std::string listenAddress = "127.0.0.1";
    int listenPort = 0;
    int maxConnections = 1;
};

} // namespace ucf::utilities
