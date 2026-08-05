#pragma once

#include <memory>
#include <string>
#include <ucf/Services/ServiceFactory/IServiceFactory.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}
namespace ucf::service{
class SERVICE_EXPORT ServiceFactory: public IServiceFactory
{
public:
    ServiceFactory(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ServiceFactory(const ServiceFactory&) = delete;
    ServiceFactory(ServiceFactory&&) = delete;
    ServiceFactory& operator=(const ServiceFactory&) = delete;
    ServiceFactory& operator=(ServiceFactory&&) = delete;
    ~ServiceFactory();
public:
    virtual void registerServices() override;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
};
}
