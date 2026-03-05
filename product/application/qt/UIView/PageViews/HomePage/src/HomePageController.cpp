#include "PageViews/HomePage/include/HomePageController.h"
#include "LoggerDefine/LoggerDefine.h"

HomePageController::HomePageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create HomePageController");
}

void HomePageController::init()
{
    UIVIEW_LOG_DEBUG("HomePageController::init");
}
