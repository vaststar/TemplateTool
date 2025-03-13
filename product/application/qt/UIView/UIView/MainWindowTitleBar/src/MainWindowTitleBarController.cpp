#include "MainWindowTitleBar/include/MainWindowTitleBarController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

MainWindowTitleBarController::MainWindowTitleBarController(QObject *parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create MainWindowTitleBarController");
}

QString MainWindowTitleBarController::getControllerName() const
{
    return QObject::tr("MainWindowTitleBarController");
}

void MainWindowTitleBarController::initializeController(QPointer<AppContext> appContext)
{
    mAppContext = appContext;
}

QString MainWindowTitleBarController::getTitle() const
{
    return QObject::tr("my test window title bar");
}
