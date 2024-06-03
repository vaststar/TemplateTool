#pragma once

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
 
namespace ucf{
    
class SERVICE_EXPORT INetworkServiceCallback
{
public:
    virtual ~ INetworkServiceCallback() = default;
};
}