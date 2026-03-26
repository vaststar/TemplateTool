#pragma once

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
 
namespace ucf::service{
    
class SERVICE_EXPORT INetworkServiceCallback
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