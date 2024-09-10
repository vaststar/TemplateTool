#pragma once

#include <memory>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

class MainWindowManager final
{
public:
    struct ApplicationConfig
    {
        int& argc;
        char **argv;
        commonHead::ICommonHeadFrameworkWPtr commonHeadFramework;
    };
    
public:
    MainWindowManager(const ApplicationConfig& config);
    ~MainWindowManager();
public:
    int runApp();
private:
    void createAndShowMainWindow();
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};