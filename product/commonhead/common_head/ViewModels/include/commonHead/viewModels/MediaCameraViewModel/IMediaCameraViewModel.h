#pragma once

#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IMediaCameraViewModelCallback
{
public:
    virtual ~IMediaCameraViewModelCallback() = default;
};

class COMMONHEAD_EXPORT IMediaCameraViewModel: public virtual ucf::utilities::INotificationHelper<IMediaCameraViewModelCallback>
{
public:
    virtual std::string getViewModelName() const = 0;
    virtual void openCamera() = 0;
public:
    static std::shared_ptr<IMediaCameraViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}