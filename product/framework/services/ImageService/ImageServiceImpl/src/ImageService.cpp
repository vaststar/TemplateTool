#include "ImageService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/ImageService/ImageServiceCreator.h>
#include "ImageServiceLogger.h"


namespace ucf::service{
namespace impl {
std::shared_ptr<IImageService> createImageService(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ImageService>(coreFramework);
}
} // namespace impl

ImageService::ImageService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("ImageService constructed, address: " << this);
}

ImageService::~ImageService()
{
    SERVICE_LOG_DEBUG("ImageService destroying, address: " << this);
}

std::string ImageService::getServiceName() const
{
    return "ImageService";
}

void ImageService::initService()
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }
}

void ImageService::deinitService()
{
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        coreFramework->unRegisterCallback(shared_from_this());
    }
}

void ImageService::onCoreFrameworkExit()
{
    SERVICE_LOG_INFO("ImageService::onCoreFrameworkExit()");
}
}
