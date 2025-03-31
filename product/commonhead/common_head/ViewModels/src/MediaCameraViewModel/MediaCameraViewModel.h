#pragma once

#include <thread>
#include <memory>
#include <string>
#include <atomic>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <ucf/Services/ImageService/ImageTypes.h>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

namespace commonHead::viewModels{
class MediaCameraViewModel: public virtual IMediaCameraViewModel, 
                           public virtual ucf::utilities::NotificationHelper<IMediaCameraViewModelCallback>,
                           public std::enable_shared_from_this<MediaCameraViewModel>
{
public:
    explicit MediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    ~MediaCameraViewModel();
    virtual std::string getViewModelName() const override;

    virtual void openCamera() override;
    virtual void startCaptureCameraVideo() override;
    virtual void stopCaptureCameraVideo() override;
private:
    model::Image convertServiceImageToViewModelImage(const ucf::service::model::Image& image) const;
private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
    std::string mCameraId;
    std::shared_ptr<std::thread> mCaptureThread;
    std::atomic<bool> mVideoCaptureStop;
};
}