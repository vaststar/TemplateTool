#pragma once

#include <string>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>
#include <ucf/Services/MediaService/IMediaService.h>
#include <ucf/Services/MediaService/MediaTypes.h>

namespace commonHead::viewModels {

class MediaCameraViewModel
    : public virtual IMediaCameraViewModel
    , public virtual commonHead::utilities::VMNotificationHelper<IMediaCameraViewModelCallback>
{
public:
    explicit MediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    MediaCameraViewModel() = delete;
    MediaCameraViewModel(const MediaCameraViewModel&) = delete;
    MediaCameraViewModel(MediaCameraViewModel&&) = delete;
    MediaCameraViewModel& operator=(const MediaCameraViewModel&) = delete;
    MediaCameraViewModel& operator=(MediaCameraViewModel&&) = delete;
    ~MediaCameraViewModel() override;

public:
    std::string getViewModelName() const override;
    void openCamera(const model::CameraSource& source) override;
    [[nodiscard]] bool isOpened() const override;
    void startCaptureCameraVideo() override;
    void stopCaptureCameraVideo() override;

protected:
    void init() override;

private:
    model::VideoFrame convertServiceFrameToViewModelFrame(
        const ucf::service::media::IVideoFramePtr& frame) const;

private:
    std::string mCameraId;
    std::string mSubscriptionId;
};

} // namespace commonHead::viewModels
