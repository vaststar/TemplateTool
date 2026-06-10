#pragma once

#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/MediaCameraViewModel/VideoFrame.h>
#include <commonHead/viewModels/MediaCameraViewModel/CameraSource.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IMediaCameraViewModelCallback
{
public:
    IMediaCameraViewModelCallback() = default;
    IMediaCameraViewModelCallback(const IMediaCameraViewModelCallback&) = delete;
    IMediaCameraViewModelCallback(IMediaCameraViewModelCallback&&) = delete;
    IMediaCameraViewModelCallback& operator=(const IMediaCameraViewModelCallback&) = delete;
    IMediaCameraViewModelCallback& operator=(IMediaCameraViewModelCallback&&) = delete;
    virtual ~IMediaCameraViewModelCallback() = default;
public:
    virtual void onCameraFrameReceived(const model::VideoFrame& /*frame*/) {};
    // Fired when openCamera() finishes without acquiring a valid handle.
    virtual void onCameraOpenFailed() {};
};

class COMMONHEAD_EXPORT IMediaCameraViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IMediaCameraViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IMediaCameraViewModel(const IMediaCameraViewModel&) = delete;
    IMediaCameraViewModel(IMediaCameraViewModel&&) = delete;
    IMediaCameraViewModel& operator=(const IMediaCameraViewModel&) = delete;
    IMediaCameraViewModel& operator=(IMediaCameraViewModel&&) = delete;
    virtual ~IMediaCameraViewModel() = default;
public:
    virtual std::string getViewModelName() const = 0;

    // 打开摄像头。每个实例只能绑一个源；要换源请重建实例。
    virtual void openCamera(const model::CameraSource& source) = 0;
    virtual bool isOpened() const = 0;

    virtual void startCaptureCameraVideo() = 0;
    virtual void stopCaptureCameraVideo() = 0;
public:
    static std::shared_ptr<IMediaCameraViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}
