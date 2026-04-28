#include "UIViewCommon/UIViewBase/include/UIViewControllerInitializer.h"

#include <AppContext/AppContext.h>

#include "UIViewCommon/UIViewBase/include/UIViewController.h"

UIViewControllerInitializer::UIViewControllerInitializer(QPointer<AppContext> appContext, QObject* parent)
    : QObject(parent)
    , mAppContext(appContext)
{
}

void UIViewControllerInitializer::initializeController(UIViewController* controller)
{
    if (controller)
    {
        controller->initializeController(mAppContext);
    }
}