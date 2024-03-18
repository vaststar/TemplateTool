#include "ImageService.h"
#include "ICoreFramework.h"

ImageService::ImageService(std::weak_ptr<ICoreFramework> coreFramework)
{
}

ImageService::~ImageService()
{
}

std::string ImageService::getServiceName() const
{
    return "ImageService";
}