#include "ImageService.h"
#include <ucf/CoreFramework/ICoreFramework.h>


namespace ucf{
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

void ImageService::onCoreFrameworkExit()
{
    
}
}