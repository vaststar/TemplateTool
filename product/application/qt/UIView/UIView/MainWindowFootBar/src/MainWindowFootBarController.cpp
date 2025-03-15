#include "MainWindowFootBar/include/MainWindowFootBarController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

MainWindowFootBarController::MainWindowFootBarController(QObject *parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create MainWindowFootBarController");
}

QString MainWindowFootBarController::getControllerName() const
{
    return QObject::tr("MainWindowFootBarController");
}

void MainWindowFootBarController::initializeController(QPointer<AppContext> appContext)
{
    mAppContext = appContext;
}

QString MainWindowFootBarController::getFooterName() const
{
    return QObject::tr("my footer bar");
}
