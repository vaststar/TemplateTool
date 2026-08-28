#include "MainWindowFootBar/MainWindowFootBarController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine.h"

MainWindowFootBarController::MainWindowFootBarController(QObject *parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("MainWindowFootBarController constructed, address: " << this);
}

MainWindowFootBarController::~MainWindowFootBarController()
{
    UIVIEW_LOG_DEBUG("MainWindowFootBarController destroying, address: " << this);
}

void MainWindowFootBarController::init()
{
}

QString MainWindowFootBarController::getFooterName() const
{
    return QObject::tr("my footer bar");
}
