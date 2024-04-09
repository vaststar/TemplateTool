#include "BaseController/BaseController.h"
#include "LoggerDefine.h"

BaseController::BaseController(QObject *parent)
    :QObject(parent)
{
    UICOM_LOG_INFO("create BaseController, address:" << this);
}

void BaseController::setCommonHeadFramework(ICommonHeadFrameworkWPtr commonheadFramework)
{
    UICOM_LOG_INFO("set CommonHeadFramework, address:" << commonheadFramework.lock());
    mCommonHeadFrameworkWPtr = commonheadFramework;
}

ICommonHeadFrameworkWPtr BaseController::getCommonHeadFramework() const
{
    return mCommonHeadFrameworkWPtr;
}

void BaseController::registerController(BaseController* controller)
{
    if (controller)
    {
        UICOM_LOG_INFO("registerController, address:" << controller);
        controller->initController(mCommonHeadFrameworkWPtr);
    }
}