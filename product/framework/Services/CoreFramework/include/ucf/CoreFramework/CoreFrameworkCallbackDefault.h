#pragma once

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/CoreFramework/ICoreFrameworkCallback.h>

namespace ucf::framework{

class SERVICE_EXPORT CoreFrameworkCallbackDefault: public ICoreFrameworkCallback
{
public:
    virtual ~CoreFrameworkCallbackDefault() = default;
    virtual void onServiceInitialized() {};
    virtual void onCoreFrameworkExit() {};
};
}