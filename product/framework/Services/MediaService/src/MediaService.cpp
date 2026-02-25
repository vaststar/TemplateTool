#include "MediaService.h"

#include <thread>

#include <ucf/CoreFramework/ICoreFramework.h>


#include "MediaServiceLogger.h"

#include "CameraManager.h"


namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class MediaService::DataPrivate{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    std::shared_ptr<CameraManager> getCameraManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::shared_ptr<CameraManager>  mCameraManagerPtr;
};

MediaService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mCameraManagerPtr(std::make_shared<CameraManager>())
{

}

ucf::framework::ICoreFrameworkWPtr MediaService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

std::shared_ptr<CameraManager> MediaService::DataPrivate::getCameraManager() const
{
    return mCameraManagerPtr;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<IMediaService> IMediaService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<MediaService>(coreFramework);
}

MediaService::MediaService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_INFO("create MediaService, address:" << this);
}

MediaService::~MediaService()
{
    SERVICE_LOG_INFO("delete MediaService, address:" << this);
}

std::string MediaService::getServiceName() const
{
    return "MediaService";
}

void MediaService::initService()
{

}

std::string MediaService::openCamera(int cameraNum)
{
    return mDataPrivate->getCameraManager()->openCamera(cameraNum);
}

void MediaService::releaseCamera(const std::string& cameraId)
{
    mDataPrivate->getCameraManager()->releaseCamera(cameraId);
}

std::vector<std::string> MediaService::getOpenedCameras() const
{
    return mDataPrivate->getCameraManager()->getOpenedCameras();
}

std::optional<media::VideoFrame> MediaService::readImageData(const std::string& cameraId)
{
    return mDataPrivate->getCameraManager()->readImageData(cameraId);
}
}