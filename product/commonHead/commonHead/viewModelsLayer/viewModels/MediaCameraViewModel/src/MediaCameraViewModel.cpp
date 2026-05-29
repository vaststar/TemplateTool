#include "MediaCameraViewModel.h"

#include <variant>

#include <ucf/Services/MediaService/IMediaService.h>
#include <ucf/Services/MediaService/MediaTypes.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

namespace commonHead::viewModels {

namespace {

ucf::service::media::CameraSource toServiceSource(const model::CameraSource& vm)
{
    return std::visit([](auto&& s) -> ucf::service::media::CameraSource {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, model::LocalCameraSource>)
        {
            return ucf::service::media::LocalCameraSource{s.index};
        }
        else
        {
            return ucf::service::media::NetworkCameraSource{
                s.url, s.transport, s.openTimeoutMs, s.readTimeoutMs};
        }
    }, vm);
}

}

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

void MediaCameraViewModel::openCamera(const model::CameraSource& source)
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto mediaService = serviceLocator->getMediaService().lock())
            {
                mCameraId = mediaService->openCamera(toServiceSource(source));
            }
        }
    }
}

bool MediaCameraViewModel::isOpened() const
{
    return !mCameraId.empty();
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
