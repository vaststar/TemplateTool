#include "BaseController/BaseController.h"
#include "LoggerDefine.h"
#include "ClientGlobal/ClientGlobal.h"

BaseController::BaseController(QObject *parent)
    :QObject(parent)
{
    setCommonHeadFramework(ClientGlobal::getInstance()->getCommonHeadFramework());
}

void BaseController::setCommonHeadFramework(ICommonHeadFrameworkWPtr commonheadFramework)
{
    mCommonHeadFrameworkWPtr = commonheadFramework;
}

ICommonHeadFrameworkWPtr BaseController::getCommonHeadFramework() const
{
    return mCommonHeadFrameworkWPtr;
}
