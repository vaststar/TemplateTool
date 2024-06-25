#include "ImageService.h"
#include <ucf/CoreFramework/ICoreFramework.h>


namespace ucf::service{
ImageService::ImageService(ucf::framework::ICoreFrameworkWPtr coreFramework)
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
}