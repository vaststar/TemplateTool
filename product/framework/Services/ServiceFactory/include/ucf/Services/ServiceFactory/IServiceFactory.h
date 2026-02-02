#pragma once

#include <ucf/Services/ServiceDeclaration/IService.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
    class IInvocationService;
    class IDataWarehouseService;
    class INetworkService;
    class IClientInfoService;
    class IContactService;
    class IImageService;
    class IMediaService;
    class IStabilityService;
}

namespace ucf::service{
class SERVICE_EXPORT IServiceFactory
{
public:
    IServiceFactory() = default;
    IServiceFactory(const IServiceFactory&) = delete;
    IServiceFactory(IServiceFactory&&) = delete;
    IServiceFactory& operator=(const IServiceFactory&) = delete;
    IServiceFactory& operator=(IServiceFactory&&) = delete;
    virtual ~IServiceFactory() = default;
public:
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IInvocationService> createInvocationService() = 0;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IDataWarehouseService> createDataWarehouseService() = 0;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::INetworkService> createNetworkService() = 0;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IClientInfoService> createClientInfoService() = 0;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IContactService> createContactService() = 0;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IImageService> createImageService() = 0;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IMediaService> createMediaService() = 0;
    [[nodiscard]] virtual std::shared_ptr<ucf::service::IStabilityService> createStabilityService() = 0;
public:
    static std::shared_ptr<IServiceFactory> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}