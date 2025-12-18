#include "UIViewBase/include/UIViewControllerInitializer.h"

#include <AppContext/AppContext.h>

#include "UIViewBase/include/UIViewController.h"

UIViewControllerInitializer::UIViewControllerInitializer(const QPointer<AppContext>& appContext, QObject* parent)
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