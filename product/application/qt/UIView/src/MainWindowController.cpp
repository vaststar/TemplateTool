#include "MainWindow/MainWindowController.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include "LoggerDefine.h"

MainWindowController::MainWindowController(QObject* parent)
    : BaseController(parent)
{
    UIVIEW_LOG_DEBUG("create MainWindowController");
}

QString MainWindowController::getControllerName() const
{
    return "MainWindowController";
}
