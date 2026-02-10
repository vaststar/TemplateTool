#include "UIViewBase/include/UIViewController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

UIViewController::UIViewController(QObject* parent)
    : UIAppCore::UIController(parent)
{
}

void UIViewController::initializeController(QPointer<AppContext> appContext)
{
    if (getAppContext())
    {
        UIVIEW_LOG_WARN("Controller " << getControllerName().toStdString() << " has been initialized already.");
        return;
    }

    if (appContext.isNull())
    {
        UIVIEW_LOG_ERROR("Failed to initialize Controller " << getControllerName().toStdString() << ": AppContext is null.");
        return;
    }

    UIVIEW_LOG_DEBUG("start initialize Controller: " << getControllerName().toStdString());
    mAppContext = appContext;
    init();
    emit controllerInitialized();
    UIVIEW_LOG_DEBUG("finish initialize Controller: " << getControllerName().toStdString());
}

QPointer<AppContext> UIViewController::getAppContext() const
{
    return mAppContext;
}

void UIViewController::logInfo(const QString& message)
{
    UIVIEW_LOG_INFO("[" << getControllerName().toStdString() << "] " << message.toStdString());
}