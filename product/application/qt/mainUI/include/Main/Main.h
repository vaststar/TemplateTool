#pragma once

#include <memory>
#include "MainExport.h"
#include "AppRunner/AppRunner.h"
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