#include "MainWindowSideBar/include/MainWindowSideBarController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

MainWindowSideBarController::MainWindowSideBarController(QObject *parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create MainWindowSideBarController");
}

QString MainWindowSideBarController::getControllerName() const
{
    return QObject::tr("MainWindowSideBarController");
}

void MainWindowSideBarController::initializeController(QPointer<AppContext> appContext)
{
    mAppContext = appContext;
}

QString MainWindowSideBarController::getTitle() const
{
    return QObject::tr("my test window title bar");
}
