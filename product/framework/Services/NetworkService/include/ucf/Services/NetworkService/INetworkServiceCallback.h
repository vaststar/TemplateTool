#pragma once

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
 
namespace ucf::service{
    
class SERVICE_EXPORT INetworkServiceCallback
{
public:
    virtual ~ INetworkServiceCallback() = default;
};
}