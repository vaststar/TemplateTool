#include "AppSystemTray/include/AppSystemTrayController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

AppSystemTrayController::AppSystemTrayController(QObject *parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create AppSystemTrayController");
}

QString AppSystemTrayController::getTitle() const
{
    return QObject::tr("my test window title bar");
}

void AppSystemTrayController::init()
{
    UIVIEW_LOG_DEBUG("initialize AppSystemTrayController");
}