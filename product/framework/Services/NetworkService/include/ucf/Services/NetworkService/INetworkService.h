#pragma once

#include <memory>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/CoreFramework/IService.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/Services/NetworkService/INetworkServiceCallback.h>

namespace ucf{

class ICoreFramework;
using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;

class SERVICE_EXPORT INetworkService: public IService, 
                                      public virtual ucf::utilities::INotificationHelper<INetworkServiceCallback>
{
public: 
    static std::shared_ptr<INetworkService> CreateInstance(ICoreFrameworkWPtr coreFramework);
};
}