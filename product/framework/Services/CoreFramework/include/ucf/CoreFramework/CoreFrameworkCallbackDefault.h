#pragma once

#include <ucf/CoreFramework/ICoreFrameworkCallback.h>

namespace ucf::framework{

class CoreFrameworkCallbackDefault: public ICoreFrameworkCallback
{
public:
    virtual ~CoreFrameworkCallbackDefault() = default;
    virtual void onServiceInitialized() {};
    virtual void onCoreFrameworkExit() {};
};
}