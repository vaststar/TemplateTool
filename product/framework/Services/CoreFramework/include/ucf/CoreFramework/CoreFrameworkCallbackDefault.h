#pragma once

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/CoreFramework/ICoreFrameworkCallback.h>

namespace ucf{

class SERVICE_EXPORT CoreFrameworkCallbackDefault: public ICoreFrameworkCallback
{
public:
    virtual ~CoreFrameworkCallbackDefault() = default;
    virtual void OnServiceInitialized() {};
    virtual void onCoreFrameworkExit() {};
};
}