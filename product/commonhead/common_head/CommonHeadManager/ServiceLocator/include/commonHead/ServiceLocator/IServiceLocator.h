#pragma once

#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
    class IDataWarehouseService;
    class INetworkService;
    class IClientInfoService;
    class IContactService;
    class IImageService;
    class IMediaService;
}

namespace commonHead{
class COMMONHEAD_EXPORT IServiceLocator
{
public:
    virtual ~IServiceLocator() = default;
    virtual std::weak_ptr<ucf::service::IDataWarehouseService> getDataWarehouseService() const = 0;
    virtual std::weak_ptr<ucf::service::INetworkService> getNetworkService() const = 0;
    virtual std::weak_ptr<ucf::service::IClientInfoService> getClientInfoService() const = 0;
    virtual std::weak_ptr<ucf::service::IContactService> getContactService() const = 0;
    virtual std::weak_ptr<ucf::service::IImageService> getImageService() const = 0;
    virtual std::weak_ptr<ucf::service::IMediaService> getMediaService() const = 0;
public:
    static std::shared_ptr<IServiceLocator> createInstance(ucf::framework::ICoreFrameworkWPtr coreframework);
};
}