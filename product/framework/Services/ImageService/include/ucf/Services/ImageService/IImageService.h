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
    virtual ~IImageService() = default;
    static std::shared_ptr<IImageService> CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}