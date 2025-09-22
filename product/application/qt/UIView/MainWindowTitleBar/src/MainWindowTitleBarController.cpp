#include "MainWindowTitleBar/include/MainWindowTitleBarController.h"

#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIManager/IThemeManager.h>

#include "LoggerDefine/LoggerDefine.h"

MainWindowTitleBarController::MainWindowTitleBarController(QObject *parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create MainWindowTitleBarController");
}

void MainWindowTitleBarController::init()
{
    UIVIEW_LOG_DEBUG("");
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
    if (auto appContext = getAppContext())
    {
        if (auto managerProvider = appContext->getManagerProvider())
        {
            if (auto themeManager = managerProvider->getThemeManager())
            {
                return themeManager->getUIColor(UIColorToken::ColorToken::MainWindowBackground, UIColorState::ColorState::Normal);
            }
        }
    }
    return QColor(255, 0, 0);
}