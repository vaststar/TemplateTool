#include "NetworkService.h"

#include <ucf/Utilities/TimeUtils/TimeUtils.h>

#include "NetworkServiceLogger.h"
#include "NetworkHttpManager.h"

namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class NetworkService::DataPrivate{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    std::shared_ptr<network::http::INetworkHttpManager> getNetworkHttpManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::shared_ptr<network::http::INetworkHttpManager> mNetworkHttpManagerPtr;
};

NetworkService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mNetworkHttpManagerPtr(std::make_shared<network::http::NetworkHttpManager>())
{

}

ucf::framework::ICoreFrameworkWPtr NetworkService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

std::shared_ptr<network::http::INetworkHttpManager> NetworkService::DataPrivate::getNetworkHttpManager() const
{
    return mNetworkHttpManagerPtr;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start NetworkService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<INetworkService> INetworkService::CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<NetworkService>(coreFramework);
}

NetworkService::NetworkService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create NetworkService, address:" << this);
    ucf::utilities::TimeUtils::getUTCCurrentTime();
    auto test = ucf::utilities::TimeUtils::getUTCNowInMilliseconds();
    SERVICE_LOG_DEBUG("now:" << test << ", zone:" << ucf::utilities::TimeUtils::getLocalTimeZone());

}
 
NetworkService::~NetworkService()
{
   SERVICE_LOG_DEBUG("");
}

network::http::INetworkHttpManagerWPtr NetworkService::getNetworkHttpManager()
{
    return std::weak_ptr(mDataPrivate->getNetworkHttpManager());
}

void NetworkService::initService()
{
    SERVICE_LOG_DEBUG("init NetworkService start, address:" << this);
    // if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    // {
    //     coreFramework->registerCallback(shared_from_this());
    // }
    mDataPrivate->getNetworkHttpManager()->startHttpNetwork();
    SERVICE_LOG_DEBUG("init NetworkService done, address:" << this);
}

std::string NetworkService::getServiceName() const
{
    return "NetworkService";
}

void NetworkService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("about exit NetworkService, address:" << this);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish NetworkService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}