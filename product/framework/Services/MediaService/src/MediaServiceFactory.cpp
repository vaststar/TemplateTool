#include "MediaService.h"

namespace ucf::service{
std::shared_ptr<IMediaService> IMediaService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<MediaService>(coreFramework);
}
}
