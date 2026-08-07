#pragma once

namespace ucf::service{

class INetworkServiceCallback
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
