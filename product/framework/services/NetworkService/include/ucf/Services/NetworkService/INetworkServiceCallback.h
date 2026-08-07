#pragma once

#include <ucf/Services/NetworkService/NetworkServiceExport.h>

namespace ucf::service{

class NETWORK_SERVICE_API INetworkServiceCallback
{
public:
    INetworkServiceCallback() = default;
    INetworkServiceCallback(const INetworkServiceCallback&) = delete;
    INetworkServiceCallback(INetworkServiceCallback&&) = delete;
    INetworkServiceCallback& operator=(const INetworkServiceCallback&) = delete;
    INetworkServiceCallback& operator=(INetworkServiceCallback&&) = delete;
    virtual ~ INetworkServiceCallback() = default;
};
}
