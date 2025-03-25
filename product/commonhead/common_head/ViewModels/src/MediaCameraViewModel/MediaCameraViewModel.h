#pragma once

#include <memory>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h>

namespace commonHead::viewModels{
class MediaCameraViewModel: public virtual IMediaCameraViewModel, 
                           public virtual ucf::utilities::NotificationHelper<IMediaCameraViewModelCallback>,
                           public std::enable_shared_from_this<MediaCameraViewModel>
{
public:
    MediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    virtual std::string getViewModelName() const override;

    virtual void openCamera() override;
private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}