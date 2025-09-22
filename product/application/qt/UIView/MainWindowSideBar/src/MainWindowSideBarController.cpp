#include "MainWindowSideBar/include/MainWindowSideBarController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

MainWindowSideBarController::MainWindowSideBarController(QObject *parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create MainWindowSideBarController");
}

void MainWindowSideBarController::init()
{
    UIVIEW_LOG_DEBUG("");
}

QString MainWindowSideBarController::getTitle() const
{
    return QObject::tr("my test window title bar");
}
