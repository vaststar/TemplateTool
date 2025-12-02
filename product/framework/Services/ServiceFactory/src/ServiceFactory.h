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
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IInvocationService> createInvocationService() override;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IDataWarehouseService> createDataWarehouseService() override;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::INetworkService> createNetworkService() override;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IClientInfoService> createClientInfoService() override;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IContactService> createContactService() override;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IImageService> createImageService() override;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IMediaService> createMediaService() override;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
};
}
