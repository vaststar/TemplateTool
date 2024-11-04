#pragma once

#include <ucf/Services/ServiceDeclaration/IService.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
class SERVICE_EXPORT IMediaService: public IService
{
public:
    virtual ~IMediaService() = default;
    virtual void openCamera() = 0;
    static std::shared_ptr<IMediaService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}