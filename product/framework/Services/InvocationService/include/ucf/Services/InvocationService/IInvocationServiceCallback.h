#pragma once

#include <vector>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service{


class SERVICE_EXPORT IInvocationServiceCallback
{
public:
    virtual ~ IInvocationServiceCallback() = default;
};
}