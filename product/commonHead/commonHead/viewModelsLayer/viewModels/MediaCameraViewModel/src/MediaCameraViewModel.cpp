#include "MediaCameraViewModel.h"

#include <ucf/Services/MediaService/IMediaService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

namespace commonHead::viewModels {
std::shared_ptr<IMediaCameraViewModel> IMediaCameraViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MediaCameraViewModel>(commonHeadFramework);
}

MediaCameraViewModel::MediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMediaCameraViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create MediaCameraViewModel");
}

MediaCameraViewModel::~MediaCameraViewModel()
{
    COMMONHEAD_LOG_DEBUG("");
    
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto mediaService = serviceLocator->getMediaService().lock())
            {
                if (!mSubscriptionId.empty())
                {
                    mediaService->stopVideoCapture(mCameraId, mSubscriptionId);
                }
                mediaService->releaseCamera(mCameraId);
            }
        }
    }
}

std::string MediaCameraViewModel::getViewModelName() const
{
    return "MediaCameraViewModel";
}

void MediaCameraViewModel::init()
{
}

void MediaCameraViewModel::openCamera()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto mediaService = serviceLocator->getMediaService().lock())
            {
                mCameraId = mediaService->openCamera(0);
            }
        }
    }
}

void MediaCameraViewModel::startCaptureCameraVideo()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto mediaService = serviceLocator->getMediaService().lock())
            {
                mSubscriptionId = mediaService->startVideoCapture(mCameraId,
                    [this](const ucf::service::media::IVideoFramePtr& frame)
                    {
                        fireNotification(&IMediaCameraViewModelCallback::onCameraFrameReceived,
                                        convertServiceFrameToViewModelFrame(frame));
                    });
            }
        }
    }
}

void MediaCameraViewModel::stopCaptureCameraVideo()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto mediaService = serviceLocator->getMediaService().lock())
            {
                if (!mSubscriptionId.empty())
                {
                    mediaService->stopVideoCapture(mCameraId, mSubscriptionId);
                    mSubscriptionId.clear();
                }
            }
        }
    }
}

model::VideoFrame MediaCameraViewModel::convertServiceFrameToViewModelFrame(
    const ucf::service::media::IVideoFramePtr& frame) const
{
    return model::VideoFrame{
        std::vector<uint8_t>(frame->getData(), frame->getData() + frame->getDataSize()),
        frame->getWidth(),
        frame->getHeight(),
        frame->getBytesPerLine(), 
        static_cast<model::PixelFormat>(frame->getFormat())
    };
}
}