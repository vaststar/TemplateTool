#include "MainWindowFootBar/include/MainWindowFootBarController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

MainWindowFootBarController::MainWindowFootBarController(QObject *parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create MainWindowFootBarController");
}

void MainWindowFootBarController::init()
{
    UIVIEW_LOG_DEBUG("");
}

QString MainWindowFootBarController::getFooterName() const
{
    return QObject::tr("my footer bar");
}
