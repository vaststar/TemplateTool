#include "MediaCameraViewModel.h"
#include "LoggerDefine.h"

#include <type_traits>
#include <variant>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ServiceLocator/IServiceLocator.h>
#include <commonhead/viewmodels/MediaCameraViewModel/MediaCameraViewModelCreator.h>
#include <ucf/services/MediaService/IMediaService.h>
#include <ucf/services/MediaService/MediaTypes.h>

namespace commonHead::viewModels {

namespace {

ucf::service::media::CameraSource toServiceSource(const model::CameraSource& source)
{
    return std::visit(
        [](const auto& cameraSource) -> ucf::service::media::CameraSource {
            using SourceType = std::decay_t<decltype(cameraSource)>;
            if constexpr (std::is_same_v<SourceType, model::LocalCameraSource>)
            {
                return ucf::service::media::LocalCameraSource{cameraSource.index};
            }
            else
            {
                return ucf::service::media::NetworkCameraSource{
                    cameraSource.url,
                    cameraSource.transport,
                    cameraSource.openTimeoutMs,
                    cameraSource.readTimeoutMs};
            }
        },
        source);
}

} // namespace

namespace impl {

std::shared_ptr<IMediaCameraViewModel> createMediaCameraViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MediaCameraViewModel>(commonHeadFramework);
}

} // namespace impl

MediaCameraViewModel::MediaCameraViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMediaCameraViewModel(commonHeadFramework)
{
    MEDIA_CAMERA_VIEW_MODEL_LOG_DEBUG("create MediaCameraViewModel");
}

MediaCameraViewModel::~MediaCameraViewModel()
{
    MEDIA_CAMERA_VIEW_MODEL_LOG_DEBUG("");

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

    if (mCameraId.empty())
    {
        fireNotification(&IMediaCameraViewModelCallback::onCameraOpenFailed);
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
                mSubscriptionId = mediaService->startVideoCapture(
                    mCameraId,
                    [this](const ucf::service::media::IVideoFramePtr& frame) {
                        fireNotification(
                            &IMediaCameraViewModelCallback::onCameraFrameReceived,
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
        static_cast<model::PixelFormat>(frame->getFormat())};
}

} // namespace commonHead::viewModels
