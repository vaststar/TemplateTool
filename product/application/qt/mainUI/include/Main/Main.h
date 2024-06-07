#pragma once

#include <memory>
#include "MainExport.h"
#include "AppRunner/AppRunner.h"
class Main_EXPORT Main final
{
public:
    Main();
    ~Main();
    int runMain(int argc, char *argv[]);
private:
    void initApp(int argc, char *argv[]);
    int runMainWindowExec(int argc, char *argv[]);
    void onExitApp();
public:
    Main(const Main&) = delete;
    Main(Main&&) = delete;
    Main& operator=(const Main&) = delete;
    Main& operator=(Main&&) = delete;
private:
    AppRunner::FrameworkDependencies mDependencies;
};