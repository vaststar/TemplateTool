#include "UIViewBase/include/UIViewController.h"

#include <AppContext/AppContext.h>

#include "LoggerDefine/LoggerDefine.h"

UIViewController::UIViewController(QObject* parent)
    : UICore::CoreController(parent)
{
}

void UIViewController::initializeController(const QPointer<AppContext>& appContext)
{
    UIVIEW_LOG_DEBUG("start initialize Controller: " << getControllerName().toStdString());
    mAppContext = appContext;
    init();
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