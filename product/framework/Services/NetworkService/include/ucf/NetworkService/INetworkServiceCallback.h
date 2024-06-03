#pragma once

#include <ucf/ServiceCommonFile/ServiceExport.h>
 
namespace ucf{
    
class SERVICE_EXPORT INetworkServiceCallback
{
public:
    virtual ~ INetworkServiceCallback() = default;
};
}