#include "ImageService.h"



namespace ucf::service{
std::shared_ptr<IImageService> IImageService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ImageService>(coreFramework);
}
}