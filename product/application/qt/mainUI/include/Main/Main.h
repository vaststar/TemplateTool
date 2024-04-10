#pragma once
#include "MainExport.h"
#include "AppRunner/AppRunner.h"
#include <memory>

class ICommonHeadFramework;
using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

class Main_EXPORT Main
{
public:
    Main();
    int runMain(int argc, char *argv[]);
private:
    void initMain(int argc, char *argv[]);
private:
    AppRunner::FrameworkDependencies mDependencies;
};