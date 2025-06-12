#pragma once

#include <commonHead/ServiceLocator/IServiceLocator.h>

namespace commonHead{
class ServiceLocator: public IServiceLocator
{
public:
    explicit ServiceLocator(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual std::weak_ptr<ucf::service::IInvocationService> getInvocationService() const override;
    virtual std::weak_ptr<ucf::service::IDataWarehouseService> getDataWarehouseService() const override;
    virtual std::weak_ptr<ucf::service::INetworkService> getNetworkService() const override;
    virtual std::weak_ptr<ucf::service::IClientInfoService> getClientInfoService() const override;
    virtual std::weak_ptr<ucf::service::IContactService> getContactService() const override;
    virtual std::weak_ptr<ucf::service::IImageService> getImageService() const override;
    virtual std::weak_ptr<ucf::service::IMediaService> getMediaService() const override;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
};
}
