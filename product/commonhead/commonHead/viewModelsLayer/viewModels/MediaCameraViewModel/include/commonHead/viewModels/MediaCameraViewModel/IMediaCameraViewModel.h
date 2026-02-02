#pragma once

#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/ViewModelDataDefine/Image.h>
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
    virtual void onCameraImageReceived(const model::Image& image) {};
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
    virtual void openCamera() = 0;
    virtual void startCaptureCameraVideo() = 0;
    virtual void stopCaptureCameraVideo() = 0;
public:
    static std::shared_ptr<IMediaCameraViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}