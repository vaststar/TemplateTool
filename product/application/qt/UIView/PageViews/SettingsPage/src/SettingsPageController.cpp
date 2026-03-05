#include "PageViews/SettingsPage/include/SettingsPageController.h"
#include "LoggerDefine/LoggerDefine.h"

SettingsPageController::SettingsPageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create SettingsPageController");
}

void SettingsPageController::init()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::init");
}
