#include "NetworkService.h"

namespace ucf::service{
    std::shared_ptr<INetworkService> INetworkService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
    {
        return std::make_shared<NetworkService>(coreFramework);
    }
}