#include "UIViewBase/UIViewControllerInitializer.h"

#include <AppContext/AppContext.h>

#include "UIViewBase/UIViewController.h"

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
