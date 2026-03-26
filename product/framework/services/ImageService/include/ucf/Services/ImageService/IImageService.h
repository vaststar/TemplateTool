#pragma once

#include <ucf/Services/ServiceDeclaration/IService.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
class SERVICE_EXPORT IImageService: public IService
{
public:
    IImageService() = default;
    IImageService(const IImageService&) = delete;
    IImageService(IImageService&&) = delete;
    IImageService& operator=(const IImageService&) = delete;
    IImageService& operator=(IImageService&&) = delete;
    virtual ~IImageService() = default;
public:
    static std::shared_ptr<IImageService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}