#pragma once

#include <memory>
#include <string>

#include <commonhead/utilities/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>
#include <commonhead/viewmodels/MediaCameraViewModel/CameraSource.h>
#include <commonhead/viewmodels/MediaCameraViewModel/VideoFrame.h>

namespace commonHead::viewModels {

class IMediaCameraViewModelCallback
{
public:
    IMediaCameraViewModelCallback() = default;
    IMediaCameraViewModelCallback(const IMediaCameraViewModelCallback&) = delete;
    IMediaCameraViewModelCallback(IMediaCameraViewModelCallback&&) = delete;
    IMediaCameraViewModelCallback& operator=(const IMediaCameraViewModelCallback&) = delete;
    IMediaCameraViewModelCallback& operator=(IMediaCameraViewModelCallback&&) = delete;
    virtual ~IMediaCameraViewModelCallback() = default;

public:
    virtual void onCameraFrameReceived(const model::VideoFrame& /*frame*/) {}
    virtual void onCameraOpenFailed() {}
};

class IMediaCameraViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<IMediaCameraViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IMediaCameraViewModel(const IMediaCameraViewModel&) = delete;
    IMediaCameraViewModel(IMediaCameraViewModel&&) = delete;
    IMediaCameraViewModel& operator=(const IMediaCameraViewModel&) = delete;
    IMediaCameraViewModel& operator=(IMediaCameraViewModel&&) = delete;
    virtual ~IMediaCameraViewModel() = default;

public:
    virtual std::string getViewModelName() const override = 0;

    // Each instance binds to one source. Create another instance to switch sources.
    virtual void openCamera(const model::CameraSource& source) = 0;
    [[nodiscard]] virtual bool isOpened() const = 0;

    virtual void startCaptureCameraVideo() = 0;
    virtual void stopCaptureCameraVideo() = 0;
};

} // namespace commonHead::viewModels
