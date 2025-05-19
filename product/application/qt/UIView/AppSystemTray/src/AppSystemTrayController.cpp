#include "AppSystemTray/include/AppSystemTrayController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

AppSystemTrayController::AppSystemTrayController(QObject *parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create AppSystemTrayController");
}

QString AppSystemTrayController::getControllerName() const
{
    return QObject::tr("AppSystemTrayController");
}

void AppSystemTrayController::initializeController(QPointer<AppContext> appContext)
{
    mAppContext = appContext;
}

QString AppSystemTrayController::getTitle() const
{
    return QObject::tr("my test window title bar");
}
