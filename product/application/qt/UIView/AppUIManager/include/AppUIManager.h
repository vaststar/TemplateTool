#pragma once

#include <memory>
#include <AppUIManager/include/AppUIExport.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class AppUI_EXPORT AppUIManager final
{
public:
    struct ApplicationConfig
    {
        int& argc;
        char **argv;
        commonHead::ICommonHeadFrameworkWPtr commonHeadFramework;
    };
    
public:
    AppUIManager(const ApplicationConfig& config);
    ~AppUIManager();
public:
    int runApp();
private:
    void createAndShowMainWindow();
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};