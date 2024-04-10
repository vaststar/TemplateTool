#include "BaseController/BaseController.h"
#include "LoggerDefine.h"
#include "ClientGlobal/ClientGlobal.h"

BaseController::BaseController(QObject *parent)
    :QObject(parent)
{
    UICOM_LOG_INFO("create BaseController, address:" << this);
    setCommonHeadFramework(ClientGlobal::getInstance()->getCommonHeadFramework());
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
