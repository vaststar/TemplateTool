#include "ImageService.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include "ImageServiceLogger.h"


namespace ucf::service{

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

}
}