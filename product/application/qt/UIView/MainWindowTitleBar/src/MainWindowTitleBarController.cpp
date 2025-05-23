#include "MainWindowTitleBar/include/MainWindowTitleBarController.h"

#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIManager/IThemeManager.h>

#include "LoggerDefine/LoggerDefine.h"

MainWindowTitleBarController::MainWindowTitleBarController(QObject *parent)
    : CoreController(parent)
    , mAppContext(nullptr)
{
    UIVIEW_LOG_DEBUG("create MainWindowTitleBarController");
}

QString MainWindowTitleBarController::getControllerName() const
{
    return QObject::tr("MainWindowTitleBarController");
}

void MainWindowTitleBarController::initializeController(QPointer<AppContext> appContext)
{
    mAppContext = appContext;
}

QString MainWindowTitleBarController::getTitle() const
{
    return QObject::tr("my test window title bar");
}

bool MainWindowTitleBarController::isVisible() const
{
    return true;
}

QColor MainWindowTitleBarController::getColor() const
{
    if (mAppContext && mAppContext->getManagerProvider())
    {
        if (auto themeManager = mAppContext->getManagerProvider()->getThemeManager())
        {
            return themeManager->getUIColor(UIElementData::UIColorEnum::UIColorEnum_MAIN_WINDOW_BACKGROUND, UIElementData::UIColorState::UIColorState_Normal);
        }
    }
    return QColor(255, 0, 0);
}