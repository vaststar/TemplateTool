#include "MediaCameraViewModel.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <ucf/Services/MediaService/IMediaService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

namespace commonHead::viewModels{
std::shared_ptr<IMediaCameraViewModel> IMediaCameraViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MediaCameraViewModel>(commonHeadFramework);
}

MediaCameraViewModel::MediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create MediaCameraViewModel");
}

std::string MediaCameraViewModel::getViewModelName() const
{
    return "MediaCameraViewModel";
}

void MediaCameraViewModel::openCamera()
{
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto media = coreFramework->getService<ucf::service::IMediaService>().lock())
        {
            media->openCamera(0);
        }
    }
}
}