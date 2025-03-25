#pragma once

#include <vector>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service{
class SERVICE_EXPORT IMediaServiceCallback
{
public:
    virtual ~ IMediaServiceCallback() = default;
};
}