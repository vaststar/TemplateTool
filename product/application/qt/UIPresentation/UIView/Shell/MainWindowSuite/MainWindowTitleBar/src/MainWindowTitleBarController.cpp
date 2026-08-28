#include "MainWindowTitleBar/MainWindowTitleBarController.h"

#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>

#include "LoggerDefine.h"

MainWindowTitleBarController::MainWindowTitleBarController(QObject *parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("MainWindowTitleBarController constructed, address: " << this);
}

MainWindowTitleBarController::~MainWindowTitleBarController()
{
    UIVIEW_LOG_DEBUG("MainWindowTitleBarController destroying, address: " << this);
}

void MainWindowTitleBarController::init()
{
    emit elementUpdated();
}

QString MainWindowTitleBarController::getTitle() const
{
    return QObject::tr("oo window title bar");
}

bool MainWindowTitleBarController::isVisible() const
{
    return true;
}

QColor MainWindowTitleBarController::getColor() const
{
    if (auto resourceLoaderManager = getResourceLoaderManager())
    {
        return resourceLoaderManager->getUIColor(UIColorToken::ColorToken::Main_Window_Background, UIColorState::ColorState::Normal);
    }
    return QColor(255, 0, 0);
}
