#pragma once

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::framework{

class SERVICE_EXPORT ICoreFrameworkCallback
{
public:
    virtual ~ICoreFrameworkCallback() = default;
    virtual void onServiceInitialized() = 0;
    virtual void onCoreFrameworkExit() = 0;
};
}