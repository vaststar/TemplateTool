#pragma once

#include <string>
#include <memory>
#include <vector>

#include "ServiceCommonFile/ServiceExport.h"
#include "CoreFramework/ServiceAccessor.h"
#include "NotificationHelper/NotificationHelper.h"
#include "CoreFramework/ICoreFrameworkCallback.h"

class IService;

class SERVICE_EXPORT ICoreFramework: public ServiceAccessor, public NotificationHelper<ICoreFrameworkCallback>
{
public:
    virtual ~ICoreFramework() = default;
public:
    virtual std::string getName() const = 0;
    virtual void initServices() = 0;
    
    static std::shared_ptr<ICoreFramework> CreateInstance();
};

using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;