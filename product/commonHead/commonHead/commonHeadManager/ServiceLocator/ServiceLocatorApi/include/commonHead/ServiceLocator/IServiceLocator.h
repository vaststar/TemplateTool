#pragma once

#include <memory>

namespace ucf::service{
class IInvocationService;
class IDataWarehouseService;
class INetworkService;
class IClientInfoService;
class IContactService;
class ICameraDirectoryService;
class IImageService;
class IMediaService;
class IStabilityService;
class IPerformanceService;
class IFeatureSettingsService;
class IUpgradeService;
class IMiniAppService;
}

namespace commonHead{
class IServiceLocator
{
public:
    virtual ~IServiceLocator() = default;

    [[nodiscard]] virtual std::weak_ptr<ucf::service::IInvocationService> getInvocationService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IDataWarehouseService> getDataWarehouseService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::INetworkService> getNetworkService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IClientInfoService> getClientInfoService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IContactService> getContactService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::ICameraDirectoryService> getCameraDirectoryService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IImageService> getImageService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IMediaService> getMediaService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IStabilityService> getStabilityService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IPerformanceService> getPerformanceService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IFeatureSettingsService> getFeatureSettingsService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IUpgradeService> getUpgradeService() const = 0;
    [[nodiscard]] virtual std::weak_ptr<ucf::service::IMiniAppService> getMiniAppService() const = 0;
};
}
