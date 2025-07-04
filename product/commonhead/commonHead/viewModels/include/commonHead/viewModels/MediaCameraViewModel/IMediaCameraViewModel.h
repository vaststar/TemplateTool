#pragma once

#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/ViewModelDataDefine/Image.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IMediaCameraViewModelCallback
{
public:
    virtual ~IMediaCameraViewModelCallback() = default;
    virtual void onCameraImageReceived(const model::Image& image) {};
};

class COMMONHEAD_EXPORT IMediaCameraViewModel: public virtual commonHead::utilities::IVMNotificationHelper<IMediaCameraViewModelCallback>
{
public:
    virtual std::string getViewModelName() const = 0;
    virtual void openCamera() = 0;
    virtual void startCaptureCameraVideo() = 0;
    virtual void stopCaptureCameraVideo() = 0;
public:
    static std::shared_ptr<IMediaCameraViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}