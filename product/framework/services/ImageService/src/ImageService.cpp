#include "ImageService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include "ImageServiceLogger.h"


namespace ucf::service{
std::shared_ptr<IImageService> IImageService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ImageService>(coreFramework);
}

ImageService::ImageService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_INFO("create ImageService, address:" << this);
}

ImageService::~ImageService()
{
    SERVICE_LOG_INFO("delete ImageService, address:" << this);
}

std::string ImageService::getServiceName() const
{
    return "ImageService";
}

void ImageService::initService()
{
    SERVICE_LOG_INFO("ImageService::initService()");
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }
}

void ImageService::deinitService()
{
    SERVICE_LOG_INFO("ImageService::deinitService()");
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
