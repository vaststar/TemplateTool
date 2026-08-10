#pragma once

#include <ucf/CoreFramework/ICoreFrameworkCallback.h>

namespace ucf::framework{

class CoreFrameworkCallbackDefault: public ICoreFrameworkCallback
{
public:
    CoreFrameworkCallbackDefault() = default;
    CoreFrameworkCallbackDefault(const CoreFrameworkCallbackDefault&) = delete;
    CoreFrameworkCallbackDefault(CoreFrameworkCallbackDefault&&) = delete;
    CoreFrameworkCallbackDefault& operator=(const CoreFrameworkCallbackDefault&) = delete;
    CoreFrameworkCallbackDefault& operator=(CoreFrameworkCallbackDefault&&) = delete;
    virtual ~CoreFrameworkCallbackDefault() = default;
public:
    virtual void onServiceInitialized() {};
    virtual void onCoreFrameworkExit() {};
};
}