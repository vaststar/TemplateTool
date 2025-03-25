#pragma once

#include <optional>
#include <vector>
#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/ImageService/ImageTypes.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service::model{
struct Image;
}

namespace ucf::service{
class SERVICE_EXPORT IMediaService: public IService
{
public:
    virtual ~IMediaService() = default;
    static std::shared_ptr<IMediaService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
public:
    virtual std::string openCamera(int cameraNum) = 0;
    virtual std::vector<std::string> getOpenedCameras() const = 0;
    virtual std::optional<model::Image> readImageData(const std::string& cameraId) = 0;
};
}