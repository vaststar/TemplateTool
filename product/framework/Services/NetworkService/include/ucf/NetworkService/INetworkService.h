#pragma once

#include <memory>

#include <ucf/ServiceCommonFile/ServiceExport.h>
#include <ucf/CoreFramework/IService.h>
#include <Utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/NetworkService/INetworkServiceCallback.h>

namespace ucf{
    
class ICoreFramework;
using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;

class SERVICE_EXPORT INetworkService: public IService, 
                                      public virtual Utilities::INotificationHelper<INetworkServiceCallback>
{
public: 
    static std::shared_ptr<INetworkService> CreateInstance(ICoreFrameworkWPtr coreFramework);
};
}