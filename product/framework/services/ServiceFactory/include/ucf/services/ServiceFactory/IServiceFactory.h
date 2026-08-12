#pragma once

#include <ucf/services/ServiceDeclaration/IService.h>
#include <ucf/services/ServiceFactory/ServiceFactoryExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
class SERVICE_FACTORY_API IServiceFactory
{
public:
    IServiceFactory() = default;
    IServiceFactory(const IServiceFactory&) = delete;
    IServiceFactory(IServiceFactory&&) = delete;
    IServiceFactory& operator=(const IServiceFactory&) = delete;
    IServiceFactory& operator=(IServiceFactory&&) = delete;
    virtual ~IServiceFactory() = default;
public:
    // Creates every application service and registers it into the ICoreFramework
    // this factory was constructed with. Call after ICoreFramework::initCoreFramework().
    virtual void createServices() = 0;
public:
    static std::shared_ptr<IServiceFactory> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}
