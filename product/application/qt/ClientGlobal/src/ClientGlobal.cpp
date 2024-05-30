#include "ClientGlobal/ClientGlobal.h"
#include "ClientGlobal/LoggerDefine.h"

#include <mutex>
std::shared_ptr<ClientGlobal> ClientGlobal::_instance = nullptr;
std::shared_ptr<ClientGlobal> ClientGlobal::getInstance()
{
    static std::once_flag s_flag;
    std::call_once(s_flag, [&]() {
        CLIENTGLOBAL_LOG_DEBUG("init ClientGloabal");
        _instance.reset(new ClientGlobal());
    });
    return _instance;
}

ClientGlobal::~ClientGlobal()
{
}

void ClientGlobal::setCommonHeadFramework(commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    CLIENTGLOBAL_LOG_DEBUG("init CommonheadFramework, address: " << commonheadFramework.lock());
    mCommonHeadFramework = commonheadFramework;
}

commonHead::ICommonHeadFrameworkWPtr ClientGlobal::getCommonHeadFramework() const
{
    return mCommonHeadFramework;
}