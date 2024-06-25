#pragma once

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
 
namespace ucf::service{
    
class SERVICE_EXPORT INetworkServiceCallback
{
public:
    virtual ~ INetworkServiceCallback() = default;
};
}