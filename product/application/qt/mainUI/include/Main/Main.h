#pragma once

#include <memory>
#include "MainExport.h"
#include "AppRunner/AppRunner.h"
class Main_EXPORT Main
{
public:
    Main();
    ~Main();
    Main(const Main&) = delete;
    Main(Main&&) = delete;
    Main& operator=(const Main&) = delete;
    Main& operator=(Main&&) = delete;
    int runMain(int argc, char *argv[]);
private:
    void initMain(int argc, char *argv[]);
    void onExitApp();
private:
    AppRunner::FrameworkDependencies mDependencies;
};