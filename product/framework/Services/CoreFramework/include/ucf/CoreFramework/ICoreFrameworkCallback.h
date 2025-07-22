#pragma once

namespace ucf::framework{

class ICoreFrameworkCallback
{
public:
    virtual ~ICoreFrameworkCallback() = default;
    virtual void onServiceInitialized() = 0;
    virtual void onCoreFrameworkExit() = 0;
};
}