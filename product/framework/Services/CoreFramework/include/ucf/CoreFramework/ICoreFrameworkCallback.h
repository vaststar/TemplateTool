#pragma once

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>

namespace ucf::framework{

class SERVICE_EXPORT ICoreFrameworkCallback
{
public:
    virtual ~ICoreFrameworkCallback() = default;
    virtual void OnServiceInitialized() = 0;
    virtual void onCoreFrameworkExit() = 0;
};
}