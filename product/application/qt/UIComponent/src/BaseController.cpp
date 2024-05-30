#include "BaseController/BaseController.h"
#include "LoggerDefine.h"
#include "ClientGlobal/ClientGlobal.h"

BaseController::BaseController(QObject *parent)
    :QObject(parent)
{
    setCommonHeadFramework(ClientGlobal::getInstance()->getCommonHeadFramework());
}

void BaseController::setCommonHeadFramework(commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    mCommonHeadFrameworkWPtr = commonheadFramework;
}

commonHead::ICommonHeadFrameworkWPtr BaseController::getCommonHeadFramework() const
{
    return mCommonHeadFrameworkWPtr;
}
