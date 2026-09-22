#pragma once

#include <string>
#include <memory>

#include <ucf/CoreFramework/IServiceAccessor.h>
#include <ucf/CoreFramework/ICoreFrameworkCallback.h>

#include <ucf/utilities/NotificationHelper/INotificationHelper.h>

namespace ucf::framework{

class IService;

class ICoreFramework: public virtual IServiceAccessor, public virtual ucf::utilities::INotificationHelper<ICoreFrameworkCallback>
{
public:
    ICoreFramework() = default;
    ICoreFramework(const ICoreFramework&) = delete;
    ICoreFramework(ICoreFramework&&) = delete;
    ICoreFramework& operator=(const ICoreFramework&) = delete;
    ICoreFramework& operator=(ICoreFramework&&) = delete;
    virtual ~ICoreFramework() = default;
public:
    virtual std::string getName() const = 0;
    virtual void initCoreFramework() = 0;
    virtual void initServices() = 0;
    virtual void exitCoreFramework() = 0;
};

using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
}
