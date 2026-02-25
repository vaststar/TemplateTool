#pragma once

#include <thread>
#include <memory>
#include <string>
#include <atomic>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <ucf/Services/MediaService/MediaTypes.h>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

namespace commonHead::viewModels{
class MediaCameraViewModel: public virtual IMediaCameraViewModel, 
                            public virtual commonHead::utilities::VMNotificationHelper<IMediaCameraViewModelCallback>,
                            public std::enable_shared_from_this<MediaCameraViewModel>
{
public:
    explicit MediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    MediaCameraViewModel() = default;
    MediaCameraViewModel(const MediaCameraViewModel&) = delete;
    MediaCameraViewModel(MediaCameraViewModel&&) = delete;
    MediaCameraViewModel& operator=(const MediaCameraViewModel&) = delete;
    MediaCameraViewModel& operator=(MediaCameraViewModel&&) = delete;
    ~MediaCameraViewModel();
public:
    virtual std::string getViewModelName() const override;

    virtual void openCamera() override;
    virtual void startCaptureCameraVideo() override;
    virtual void stopCaptureCameraVideo() override;
protected:
    virtual void init() override;
private:
    model::VideoFrame convertServiceFrameToViewModelFrame(const ucf::service::media::VideoFrame& frame) const;
private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
    std::string mCameraId;
    std::shared_ptr<std::thread> mCaptureThread;
    std::atomic<bool> mVideoCaptureStop;
};
}