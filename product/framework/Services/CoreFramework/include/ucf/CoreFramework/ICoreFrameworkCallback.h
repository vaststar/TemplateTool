#pragma once

namespace ucf::framework{

class ICoreFrameworkCallback
{
public:
    ICoreFrameworkCallback() = default;
    ICoreFrameworkCallback(const ICoreFrameworkCallback&) = delete;
    ICoreFrameworkCallback(ICoreFrameworkCallback&&) = delete;
    ICoreFrameworkCallback& operator=(const ICoreFrameworkCallback&) = delete;
    ICoreFrameworkCallback& operator=(ICoreFrameworkCallback&&) = delete;
    virtual ~ICoreFrameworkCallback() = default;
public:
    virtual void onServiceInitialized() = 0;
    virtual void onCoreFrameworkExit() = 0;
};
}