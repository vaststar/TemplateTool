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
    SERVICE_LOG_INFO("MediaService::initService()");
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }
}

void MediaService::deinitService()
{
    SERVICE_LOG_INFO("MediaService::deinitService()");
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->unRegisterCallback(shared_from_this());
    }
}

void MediaService::onCoreFrameworkExit()
{
    SERVICE_LOG_INFO("MediaService::onCoreFrameworkExit()");
}

std::string MediaService::openCamera(const media::CameraSource& source)
{
    return mDataPrivate->getCameraManager()->openCamera(source);
}

void MediaService::releaseCamera(const std::string& cameraId)
{
    mDataPrivate->getCameraManager()->releaseCamera(cameraId);
}

std::vector<std::string> MediaService::getOpenedCameras() const
{
    return mDataPrivate->getCameraManager()->getOpenedCameras();
}

media::IVideoFramePtr MediaService::readImageData(const std::string& cameraId)
{
    return mDataPrivate->getCameraManager()->readImageData(cameraId);
}

std::string MediaService::startVideoCapture(
    const std::string& cameraId,
    VideoFrameCallback callback)
{
    return mDataPrivate->getCameraManager()->startVideoCapture(cameraId, std::move(callback));
}

void MediaService::stopVideoCapture(
    const std::string& cameraId,
    const std::string& subscriptionId)
{
    mDataPrivate->getCameraManager()->stopVideoCapture(cameraId, subscriptionId);
}
}
