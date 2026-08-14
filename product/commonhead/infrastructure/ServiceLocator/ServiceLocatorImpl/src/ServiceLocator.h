#pragma once

#include <memory>

#include <commonhead/ServiceLocator/IServiceLocator.h>

namespace ucf::framework{
class ICoreFramework;
}

namespace commonHead{
class ServiceLocator final : public IServiceLocator
{
public:
    explicit ServiceLocator(std::weak_ptr<ucf::framework::ICoreFramework> coreFramework);

    [[nodiscard]] std::weak_ptr<ucf::service::IInvocationService> getInvocationService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IDataWarehouseService> getDataWarehouseService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::INetworkService> getNetworkService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IClientInfoService> getClientInfoService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IContactService> getContactService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::ICameraDirectoryService> getCameraDirectoryService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IImageService> getImageService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IMediaService> getMediaService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IStabilityService> getStabilityService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IPerformanceService> getPerformanceService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IFeatureSettingsService> getFeatureSettingsService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IUpgradeService> getUpgradeService() const override;
    [[nodiscard]] std::weak_ptr<ucf::service::IMiniAppService> getMiniAppService() const override;

private:
    std::weak_ptr<ucf::framework::ICoreFramework> mCoreFramework;
};
}
