#include "MediaCameraViewModel.h"


#include <ucf/Services/MediaService/IMediaService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

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
    
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto mediaService = serviceLocator->getMediaService().lock())
            {
                mediaService->releaseCamera(mCameraId);
            }
        }
    }
}

MediaCameraViewModel::MediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMediaCameraViewModel(commonHeadFramework)
    , mVideoCaptureStop(false)
    , mCaptureThread(nullptr)
{
    COMMONHEAD_LOG_DEBUG("create MediaCameraViewModel");
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
    mCaptureThread = std::make_shared<std::thread>([this](){
        if (auto commonHeadFramework = getCommonHeadFramework().lock())
        {
            if (auto serviceLocator = commonHeadFramework->getServiceLocator())
            {
                if (auto mediaService = serviceLocator->getMediaService().lock())
                {
                    constexpr auto targetFrameTime = std::chrono::milliseconds(33);  // ~30fps
                    while(!mVideoCaptureStop)
                    {
                        auto frameStart = std::chrono::steady_clock::now();
                        
                        if (auto image = mediaService->readImageData(mCameraId))
                        {
                            fireNotification(&IMediaCameraViewModelCallback::onCameraFrameReceived, convertServiceFrameToViewModelFrame(image.value()));
                        }
                        
                        auto elapsed = std::chrono::steady_clock::now() - frameStart;
                        if (elapsed < targetFrameTime)
                        {
                            std::this_thread::sleep_for(targetFrameTime - elapsed);
                        }
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

model::VideoFrame MediaCameraViewModel::convertServiceFrameToViewModelFrame(const ucf::service::media::VideoFrame& frame) const
{
    return model::VideoFrame{frame.data, frame.width, frame.height, frame.bytesPerLine, 
                             static_cast<model::PixelFormat>(frame.format)};
}
}