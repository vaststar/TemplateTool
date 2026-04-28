#include "MainWindowSuite/MainWindowSystemTray/include/MainWindowSystemTrayController.h"

#include <AppContext/AppContext.h>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIEvents/UIMainWindowEvent.h"

MainWindowSystemTrayController::MainWindowSystemTrayController(QObject *parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create MainWindowSystemTrayController");
}

QString MainWindowSystemTrayController::getTitle() const
{
    return QObject::tr("my test window title bar");
}

void MainWindowSystemTrayController::quit()
{
    UIVIEW_LOG_DEBUG("MainWindowSystemTrayController::quit");
    sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Close);
}

void MainWindowSystemTrayController::activateMainWindow()
{
    UIVIEW_LOG_DEBUG("MainWindowSystemTrayController::activateMainWindow");
    sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Show);
    sendUIEvent<UIMainWindowEvent>(UIMainWindowEvent::Action::Activate);
}

void MainWindowSystemTrayController::init()
{
    UIVIEW_LOG_DEBUG("initialize MainWindowSystemTrayController");
}
