#include "ImageService/ImageService.h"
#include "CoreFramework/ICoreFramework.h"

ImageService::ImageService(std::weak_ptr<ICoreFramework> coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{
}

ImageService::~ImageService()
{
}

std::string ImageService::getServiceName() const
{
    return "ImageService";
}

void ImageService::initService()
{

}