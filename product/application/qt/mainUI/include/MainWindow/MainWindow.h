#pragma once
#include "MainWindowExport.h"
#include <memory>
#include <QObject>

class ICommonHeadFramework;
using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;

class MainWindow_EXPORT MainWindow
{
public:
    MainWindow(ICommonHeadFrameworkWPtr commonHeadFramework);
    int runMainWindow(int argc, char *argv[]);
private:
    ICommonHeadFrameworkWPtr mCommonHeadFrameworkWPtr;
};