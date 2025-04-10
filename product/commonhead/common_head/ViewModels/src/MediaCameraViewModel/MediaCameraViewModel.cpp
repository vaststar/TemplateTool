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

MediaCameraViewModel::~MediaCameraViewModel()
{
    COMMONHEAD_LOG_DEBUG("");
    if (mCaptureThread && mCaptureThread->joinable())
    {
        stopCaptureCameraVideo();
        mCaptureThread->join();
    }
    if (auto coreFramework = mCommonHeadFrameworkWptr.lock()->getCoreFramework().lock())
    {
        if (auto media = coreFramework->getService<ucf::service::IMediaService>().lock())
        {
            media->releaseCamera(mCameraId);
        }
    }
}

MediaCameraViewModel::MediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
    , mVideoCaptureStop(false)
    , mCaptureThread(nullptr)
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
            mCameraId = media->openCamera(0);
        }
    }
}

void MediaCameraViewModel::startCaptureCameraVideo()
{
    mCaptureThread = std::make_shared<std::thread>([this](){
        if (auto commonHeadFramework = mCommonHeadFrameworkWptr.lock())
        {
            if (auto coreFramework = commonHeadFramework->getCoreFramework().lock())
            {
                if (auto mediaService  = coreFramework->getService<ucf::service::IMediaService>().lock())
                {
                    while(!mVideoCaptureStop)
                    {
                        if (auto image = mediaService->readImageData(mCameraId))
                        {
                            fireNotification(&IMediaCameraViewModelCallback::onCameraImageReceived, convertServiceImageToViewModelImage(image.value()));
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(30));
                    }
                }
            }
        }
    });
}

void MediaCameraViewModel::stopCaptureCameraVideo()
{
    mVideoCaptureStop = true;
}

model::Image MediaCameraViewModel::convertServiceImageToViewModelImage(const ucf::service::model::Image& image) const
{
    return model::Image{image.buffer, image.width, image.height, image.steps};
}
}