#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/CoreFramework/IServiceAccessor.h>
#include <ucf/CoreFramework/ICoreFrameworkCallback.h>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

namespace ucf::framework{

class IService;

class SERVICE_EXPORT ICoreFramework: public virtual IServiceAccessor, 
                                     public virtual ucf::utilities::INotificationHelper<ICoreFrameworkCallback>
{
public:
    virtual ~ICoreFramework() = default;
public:
    virtual std::string getName() const = 0;
    virtual void initServices() = 0;
    virtual void exitCoreFramework() = 0;
    static std::shared_ptr<ICoreFramework> CreateInstance();
};

using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
}