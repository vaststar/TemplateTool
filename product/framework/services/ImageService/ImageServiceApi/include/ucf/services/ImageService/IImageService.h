#pragma once

#include <memory>

#include <ucf/services/ServiceDeclaration/IService.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
class IImageService: public IService
{
public:
    IImageService() = default;
    IImageService(const IImageService&) = delete;
    IImageService(IImageService&&) = delete;
    IImageService& operator=(const IImageService&) = delete;
    IImageService& operator=(IImageService&&) = delete;
    virtual ~IImageService() = default;
};
}
